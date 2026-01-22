#include "renderer.h"
#include "../common/asset_helper.h"
#include "../undo_commands/destroy_entities_command.h"
#include "../undo_commands/change_entities_component_command.h"
#include "../undo_commands/create_entities_from_model_command.h"
#include "../undo_commands/select_asset_entities_command.h"
#include "../widgets/main_window.h"
#include <QKeySequence>
#include <QKeyEvent>
#include <QMimeData>
#include <QGuiApplication>
#include <algorithm>
#include <iterator>
#include <array>
#include <cstdint>

Renderer::Renderer(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setFocusPolicy(Qt::FocusPolicy::ClickFocus);
	setMouseTracking(true);
	setAcceptDrops(true);

	connect(&m_waitTimer, &QTimer::timeout, this, QOverload<>::of(&QWidget::update));
	connect(&globalInfo.signalEmitter, &SignalEmitter::destroyEntitySignal, this, &Renderer::onEntityDestroyed);
	connect(&globalInfo.signalEmitter, &SignalEmitter::selectEntitySignal, this, &Renderer::onEntitySelected);
	connect(&globalInfo.signalEmitter, &SignalEmitter::switchCameraProjectionSignal, this, &Renderer::onCameraProjectionSwitched);
	connect(&globalInfo.signalEmitter, &SignalEmitter::resetCameraSignal, this, &Renderer::onCameraReset);
	connect(&globalInfo.signalEmitter, &SignalEmitter::orthographicCameraToAxisSignal, this, &Renderer::onOrthographicCameraToAxisChanged);
	connect(&globalInfo.signalEmitter, &SignalEmitter::cameraGoToEntitySignal, this, &Renderer::onCameraGoToEntity);
}

Renderer::~Renderer() {
	gl.glDeleteBuffers(1, &m_globalInfo.rendererResourceManager.rendererModels["defaultCube"].primitives[0].mesh.vertexBuffer);
	gl.glDeleteBuffers(1, &m_globalInfo.rendererResourceManager.rendererModels["defaultCube"].primitives[0].mesh.indexBuffer);
	gl.glDeleteBuffers(1, &m_globalInfo.rendererResourceManager.rendererModels["cameraFrustumCube"].primitives[0].mesh.vertexBuffer);
	gl.glDeleteBuffers(1, &m_globalInfo.rendererResourceManager.rendererModels["cameraFrustumCube"].primitives[0].mesh.indexBuffer);
	gl.glDeleteBuffers(1, &m_lightBuffer);

	for (const auto& model : m_globalInfo.rendererResourceManager.rendererModels) {
		if ((model.first == "defaultCube") || (model.first == "cameraFrustumCube")) {
			continue;
		}
		for (const auto& primitive : model.second.primitives) {
			gl.glDeleteBuffers(1, &primitive.mesh.vertexBuffer);
			gl.glDeleteBuffers(1, &primitive.mesh.indexBuffer);
		}
	}

	gl.glDeleteTextures(1, &m_pickingImage);
	gl.glDeleteRenderbuffers(1, &m_pickingDepthImage);
	gl.glDeleteTextures(1, &m_outlineSoloImage);
	gl.glDeleteRenderbuffers(1, &m_outlineSoloDepthImage);
	gl.glDeleteTextures(1, &m_globalInfo.rendererResourceManager.textures["defaultDiffuseTexture"]);
	gl.glDeleteTextures(1, &m_globalInfo.rendererResourceManager.textures["defaultEmissiveTexture"]);

	for (const auto& texture : m_globalInfo.rendererResourceManager.textures) {
		if ((texture.first == "defaultDiffuseTexture") || (texture.first == "defaultEmissiveTexture")) {
			continue;
		}

		gl.glDeleteTextures(1, &texture.second);
	}

	gl.glDeleteFramebuffers(1, &m_pickingFramebuffer);
	gl.glDeleteFramebuffers(1, &m_outlineSoloFramebuffer);

	gl.glDeleteProgram(m_entityProgram);
	gl.glDeleteProgram(m_cameraFrustumProgram);
	gl.glDeleteProgram(m_gridProgram);
	gl.glDeleteProgram(m_pickingProgram);
	gl.glDeleteProgram(m_outlineSoloProgram);
	gl.glDeleteProgram(m_outlineProgram);
}

void Renderer::initializeGL() {
	gl.initializeOpenGLFunctions();
	glex.initializeOpenGLFunctions();

	std::string fullscreenVertexShaderCode = R"GLSL(
	#version 460

	out vec2 fragUV;

	void main() {
		fragUV = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
		gl_Position = vec4(fragUV * 2.0 - 1.0, 0.0, 1.0);
	}
	)GLSL";
	GLuint fullscreenVertexShader = compileShader(GL_VERTEX_SHADER, fullscreenVertexShaderCode);

	// Entity
	std::string entityVertexShaderCode = R"GLSL(
	#version 460

	in vec3 position;
	in vec3 normal;
	in vec2 uv;

	uniform mat4 viewProj;
	uniform mat4 model;

	out vec3 fragPosition;
	out vec3 fragNormal;
	out vec2 fragUV;

	void main() {
		fragPosition = vec3(model * vec4(position, 1.0));
		fragNormal = normalize((model * vec4(normal, 0.0)).xyz);
		fragUV = uv;
		gl_Position = viewProj * vec4(fragPosition, 1.0);
	}
	)GLSL";
	GLuint entityVertexShader = compileShader(GL_VERTEX_SHADER, entityVertexShaderCode);

	std::string entityFragmentShaderCode = R"GLSL(
	#version 460

	struct TriplanarUV {
		vec2 x;
		vec2 y;
		vec2 z;
	};

	const mat4 ditheringThreshold = mat4(
		1.0 / 17.0, 13.0 / 17.0, 4.0 / 17.0, 16.0 / 17.0,
		9.0 / 17.0, 5.0 / 17.0, 12.0 / 17.0, 8.0 / 17.0,
		3.0 / 17.0, 15.0 / 17.0, 2.0 / 17.0, 14.0 / 17.0,
		11.0 / 17.0, 7.0 / 17.0, 10.0 / 17.0, 6.0 / 17.0
	);

	struct Light {
		vec3 position;
		vec3 direction;
		vec3 color;
		float intensity;
		vec2 cutoff;
		float distance;
	};

	in vec3 fragPosition;
	in vec3 fragNormal;
	in vec2 fragUV;

	uniform sampler2D diffuseTextureSampler;
	uniform sampler2D emissiveTextureSampler;
	uniform float emissiveFactor;
	uniform float alphaCutoff;
	uniform bool useTriplanarMapping;
	uniform vec2 scaleUV;
	uniform vec2 offsetUV;
	uniform bool enableShading;
	restrict readonly buffer LightBuffer {
		uvec4 count;
		Light info[];
	} lights;

	out vec4 outColor;

	void main() {
		vec4 diffuseTextureSample = vec4(0.0, 0.0, 0.0, 0.0);
		vec3 emissiveTextureSample = vec3(0.0, 0.0, 0.0);

		if (!useTriplanarMapping) {
			vec2 scaleOffsetUV = (fragUV * scaleUV) + offsetUV;

			diffuseTextureSample = texture(diffuseTextureSampler, scaleOffsetUV);
			emissiveTextureSample = texture(emissiveTextureSampler, scaleOffsetUV).rgb;
		}
		else {
			TriplanarUV triplanarUV;
			triplanarUV.x = fragPosition.zy;
			triplanarUV.x.y = -triplanarUV.x.y;
			if (fragNormal.x >= 0.0) {
				triplanarUV.x.x = -triplanarUV.x.x;
			}
			triplanarUV.y = fragPosition.xz;
			if (fragNormal.y < 0.0) {
				triplanarUV.y.y = -triplanarUV.y.y;
			}
			triplanarUV.z = fragPosition.xy;
			triplanarUV.z.y = -triplanarUV.z.y;
			if (fragNormal.z < 0.0) {
				triplanarUV.z.x = -triplanarUV.z.x;
			}

			triplanarUV.x = (triplanarUV.x * scaleUV) + offsetUV;
			triplanarUV.y = (triplanarUV.y * scaleUV) + offsetUV;
			triplanarUV.z = (triplanarUV.z * scaleUV) + offsetUV;

			vec3 triplanarWeights = abs(fragNormal);
			triplanarWeights /= (triplanarWeights.x + triplanarWeights.y + triplanarWeights.z);

			diffuseTextureSample = (texture(diffuseTextureSampler, triplanarUV.x) * triplanarWeights.x) +
				(texture(diffuseTextureSampler, triplanarUV.y) * triplanarWeights.y) +
				(texture(diffuseTextureSampler, triplanarUV.z) * triplanarWeights.z);
			emissiveTextureSample = (texture(emissiveTextureSampler, triplanarUV.x).rgb * triplanarWeights.x) +
				(texture(emissiveTextureSampler, triplanarUV.y).rgb * triplanarWeights.y) +
				(texture(emissiveTextureSampler, triplanarUV.z).rgb * triplanarWeights.z);
		}

		if ((diffuseTextureSample.a < alphaCutoff) || (diffuseTextureSample.a < ditheringThreshold[int(mod(gl_FragCoord.x, 4.0))][int(mod(gl_FragCoord.y, 4.0))])) {
			discard;
		}
		outColor = vec4(0.0, 0.0, 0.0, 1.0);

		if (enableShading) {
			uint lightIndex = 0;

			// Directional Lights
			for (uint i = 0; i < lights.count.x; i++) {
				vec3 l = -lights.info[lightIndex].direction;

				float LdotN = dot(l, fragNormal);
				if (LdotN < 0.0f) {
					lightIndex++;
					continue;
				}

				outColor += vec4(diffuseTextureSample.rgb * (lights.info[lightIndex].color * lights.info[lightIndex].intensity) * LdotN, 0.0);

				lightIndex++;
			}

			// Point Lights
			for (uint i = 0; i < lights.count.y; i++) {
				vec3 l = normalize(lights.info[lightIndex].position - fragPosition);

				float LdotN = dot(l, fragNormal);
				if (LdotN < 0.0f) {
					lightIndex++;
					continue;
				}

				float distance = length(lights.info[lightIndex].position - fragPosition);
				if (distance > lights.info[lightIndex].distance) {
					lightIndex++;
					continue;
				}

				float attenuation = 1.0 / (distance * distance);
				vec3 radiance = (lights.info[lightIndex].color * lights.info[lightIndex].intensity) * attenuation;

				outColor += vec4(diffuseTextureSample.rgb * radiance * LdotN, 0.0);

				lightIndex++;
			}

			// Spot Lights
			for (uint i = 0; i < lights.count.z; i++) {
				vec3 l = normalize(lights.info[lightIndex].position - fragPosition);

				float LdotN = dot(l, fragNormal);
				if (LdotN < 0.0f) {
					lightIndex++;
					continue;
				}

				float distance = length(lights.info[lightIndex].position - fragPosition);
				if (distance > lights.info[lightIndex].distance) {
					lightIndex++;
					continue;
				}

				float theta = dot(l, -lights.info[lightIndex].direction);
				float epsilon = cos(lights.info[lightIndex].cutoff.y) - cos(lights.info[lightIndex].cutoff.x);
				float intensity = clamp((theta - cos(lights.info[lightIndex].cutoff.x)) / epsilon, 0.0, 1.0);
				intensity = 1.0 - intensity;
				vec3 radiance = (lights.info[lightIndex].color * lights.info[lightIndex].intensity) * intensity;

				outColor += vec4(diffuseTextureSample.rgb * radiance * LdotN, 0.0);

				lightIndex++;
			}

			// Ambient Lights
			for (uint i = 0; i < lights.count.w; i++) {
				outColor += vec4(diffuseTextureSample.rgb * (lights.info[lightIndex].color * lights.info[lightIndex].intensity), 0.0);

				lightIndex++;
			}
		}
		else {
			outColor = vec4(diffuseTextureSample.rgb, 1.0);
		}

		outColor += vec4(emissiveTextureSample * emissiveFactor, 0.0);

		outColor.rgb /= outColor.rgb + vec3(1.0);
	}
	)GLSL";
	GLuint entityFragmentShader = compileShader(GL_FRAGMENT_SHADER, entityFragmentShaderCode);

	m_entityProgram = compileProgram(entityVertexShader, entityFragmentShader);

	// Camera frustum
	std::string cameraFrustumVertexShaderCode = R"GLSL(
	#version 460

	in vec3 position;

	uniform mat4 viewProj;
	uniform mat4 model;

	void main() {
		gl_Position = viewProj * model * vec4(position, 1.0);
	}
	)GLSL";
	GLuint cameraFrustumVertexShader = compileShader(GL_VERTEX_SHADER, cameraFrustumVertexShaderCode);

	std::string cameraFrustumFragmentShaderCode = R"GLSL(
	#version 460

	out vec4 outColor;

	void main() {
		outColor = vec4(0.0, 1.0, 0.0, 1.0);
	}
	)GLSL";
	GLuint cameraFrustumFragmentShader = compileShader(GL_FRAGMENT_SHADER, cameraFrustumFragmentShaderCode);

	m_cameraFrustumProgram = compileProgram(cameraFrustumVertexShader, cameraFrustumFragmentShader);

	// Grid
	std::string gridVertexShaderCode = R"GLSL(
	#version 460

	uniform mat4 view;
	uniform mat4 projection;

	out vec3 nearPoint;
	out vec3 farPoint;

	vec2 plane[6] = vec2[](
		vec2(1.0, 1.0),
		vec2(-1.0, -1.0),
		vec2(-1.0, 1.0),
		vec2(-1.0, -1.0),
		vec2(1.0, 1.0),
		vec2(1.0, -1.0)
	);

	vec3 unprojectPoint(vec3 p) {
		vec4 unprojected = inverse(view) * inverse(projection) * vec4(p, 1.0);

		return unprojected.xyz / unprojected.w;
	}

	void main() {
		vec2 p = plane[gl_VertexID];

		nearPoint = unprojectPoint(vec3(p, 0.0));
		farPoint = unprojectPoint(vec3(p, 1.0));

		gl_Position = vec4(p, 0.0, 1.0);
	}
	)GLSL";
	GLuint gridVertexShader = compileShader(GL_VERTEX_SHADER, gridVertexShaderCode);

	std::string gridFragmentShaderCode = R"GLSL(
	#version 460

	in vec3 nearPoint;
	in vec3 farPoint;

	uniform mat4 viewProj;
	uniform float near;
	uniform float far;
	uniform float gridScale;

	out vec4 outColor;

	vec4 grid(vec3 p, float scale) {
		vec2 coord = p.xz * scale;
		vec2 derivative = fwidth(coord);
		vec2 g = abs(fract(coord - vec2(0.5)) - vec2(0.5)) / derivative;
		float line = min(g.x, g.y);
		float minX = min(derivative.x, 1.0);
		float minZ = min(derivative.y, 1.0);
		vec4 color = vec4(0.2, 0.2, 0.2, 1.0 - min(line, 1.0));

		if ((p.z > -0.1 * minZ) && (p.z < 0.1 * minZ)) {
			color.r = 1.0;
		}

		if ((p.x > -0.1 * minX) && (p.x < 0.1 * minX)) {
			color.b = 1.0;
		}

		return color;
	}

	float depth(vec3 p) {
		vec4 clipSpace = viewProj * vec4(p, 1.0);

		return clipSpace.z / clipSpace.w;
	}

	float linearizeDepth(float depth) {
		float linearDepth = (2.0 * near * far) / (far + near - (depth * 2.0 - 1.0) * (far - near));

		return linearDepth / far;
	}

	void main() {
		float t = -nearPoint.y / (farPoint.y - nearPoint.y);
		vec3 fragPos = nearPoint + t * (farPoint - nearPoint);
		gl_FragDepth = (gl_DepthRange.diff * depth(fragPos) + gl_DepthRange.near + gl_DepthRange.far) / 2.0;
		float fading = max(0.5 - linearizeDepth(gl_FragDepth), 0.0);

		outColor = grid(fragPos, gridScale) * float(t > 0.0);
		outColor.a *= fading;
	}
	)GLSL";
	GLuint gridFragmentShader = compileShader(GL_FRAGMENT_SHADER, gridFragmentShaderCode);

	m_gridProgram = compileProgram(gridVertexShader, gridFragmentShader);

	// Gizmo
	std::string gizmoVertexShaderCode = R"GLSL(
	#version 460

	in vec3 position;

	uniform mat4 viewProj;
	uniform mat4 model;

	void main() {
		gl_Position = viewProj * model * vec4(position, 1.0);
	}
	)GLSL";
	GLuint gizmoVertexShader = compileShader(GL_VERTEX_SHADER, gizmoVertexShaderCode);

	std::string gizmoFragmentShaderCode = R"GLSL(
	#version 460

	uniform vec3 axisColor;

	out vec4 outColor;

	void main() {
		outColor = vec4(axisColor, 1.0);
	}
	)GLSL";
	GLuint gizmoFragmentShader = compileShader(GL_FRAGMENT_SHADER, gizmoFragmentShaderCode);

	m_gizmoProgram = compileProgram(gizmoVertexShader, gizmoFragmentShader);

	// Picking
	std::string pickingVertexShaderCode = R"GLSL(
	#version 460

	in vec3 position;

	uniform mat4 viewProj;
	uniform mat4 model;

	void main() {
		gl_Position = viewProj * model * vec4(position, 1.0);
	}
	)GLSL";
	GLuint pickingVertexShader = compileShader(GL_VERTEX_SHADER, pickingVertexShaderCode);

	std::string pickingFragmentShaderCode = R"GLSL(
	#version 460

	uniform uint entityID;

	out uint outEntityID;

	void main() {
		outEntityID = entityID;
	}
	)GLSL";
	GLuint pickingFragmentShader = compileShader(GL_FRAGMENT_SHADER, pickingFragmentShaderCode);

	m_pickingProgram = compileProgram(pickingVertexShader, pickingFragmentShader);

	gl.glGenFramebuffers(1, &m_pickingFramebuffer);
	createPickingImages();

	// Outline Solo
	std::string outlineSoloVertexShaderCode = R"GLSL(
	#version 460

	in vec3 position;

	uniform mat4 viewProj;
	uniform mat4 model;

	void main() {
		gl_Position = viewProj * model * vec4(position, 1.0);
	}
	)GLSL";
	GLuint outlineSoloVertexShader = compileShader(GL_VERTEX_SHADER, outlineSoloVertexShaderCode);

	std::string outlineSoloFragmentShaderCode = R"GLSL(
	#version 460

	out float outColor;

	void main() {
		outColor = 1.0;
	}
	)GLSL";
	GLuint outlineSoloFragmentShader = compileShader(GL_FRAGMENT_SHADER, outlineSoloFragmentShaderCode);

	m_outlineSoloProgram = compileProgram(outlineSoloVertexShader, outlineSoloFragmentShader);

	gl.glGenFramebuffers(1, &m_outlineSoloFramebuffer);
	createOutlineSoloImages();

	// Outline
	std::string outlineFragmentShaderCode = R"GLSL(
	#version 460

	uniform sampler2D outlineSoloTexture;
	uniform vec3 outlineColor;

	in vec2 fragUV;

	out vec4 outColor;

	void main() {
		float value = texture(outlineSoloTexture, fragUV).r;
		if (value == 1.0) {
			discard;
		}

		vec2 texelSize = 1.0 / vec2(textureSize(outlineSoloTexture, 0));
		bool foundValue = false;
		for (float range = 0.0; range < 2.0; range++) {
			float n = texture(outlineSoloTexture, fragUV + vec2(0.0, texelSize.y * (range + 1.0))).r;
			float s = texture(outlineSoloTexture, fragUV + vec2(0.0, -texelSize.y * (range + 1.0))).r;
			float e = texture(outlineSoloTexture, fragUV + vec2(-texelSize.x * (range + 1.0), 0.0)).r;
			float w = texture(outlineSoloTexture, fragUV + vec2(texelSize.x * (range + 1.0), 0.0)).r;
			if ((n == 1.0) || (s == 1.0) || (e == 1.0) || (w == 1.0)) {
				outColor = vec4(outlineColor, 1.0);
				foundValue = true;
				break;
			}
		}
		if (!foundValue) {
			discard;
		}
	}
	)GLSL";
	GLuint outlineFragmentShader = compileShader(GL_FRAGMENT_SHADER, outlineFragmentShaderCode);

	m_outlineProgram = compileProgram(fullscreenVertexShader, outlineFragmentShader);

	// Collider
	std::string colliderVertexShaderCode = R"GLSL(
	#version 460

	in vec3 position;

	uniform mat4 viewProj;
	uniform mat4 model;

	void main() {
		gl_Position = viewProj * model * vec4(position, 1.0);
	}
	)GLSL";
	GLuint colliderVertexShader = compileShader(GL_VERTEX_SHADER, colliderVertexShaderCode);

	std::string colliderFragmentShaderCode = R"GLSL(
	#version 460

	uniform vec3 colliderColor;

	out vec4 outColor;

	void main() {
		outColor = vec4(colliderColor, 1.0f);
	}
	)GLSL";
	GLuint colliderFragmentShader = compileShader(GL_FRAGMENT_SHADER, colliderFragmentShaderCode);

	m_colliderProgram = compileProgram(colliderVertexShader, colliderFragmentShader);

	// Cube indices
	GLuint cubeTriangleIndexBuffer;
	gl.glGenBuffers(1, &cubeTriangleIndexBuffer);
	std::vector<uint32_t> cubeTriangleIndices = { 0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7, 8, 9, 10, 8, 10, 11, 12, 13, 14, 12, 14, 15, 16, 17, 18, 16, 18, 19, 20, 21, 22, 20, 22, 23 };
	gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeTriangleIndexBuffer);
	gl.glBufferData(GL_ELEMENT_ARRAY_BUFFER, cubeTriangleIndices.size() * sizeof(uint32_t), cubeTriangleIndices.data(), GL_STATIC_DRAW);

	GLuint cubeLineIndexBuffer;
	gl.glGenBuffers(1, &cubeLineIndexBuffer);
	std::vector<uint32_t> cubeLineIndices = { 0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6, 6, 7, 7, 4, 0, 4, 1, 5, 2, 6, 3, 7 };
	gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeLineIndexBuffer);
	gl.glBufferData(GL_ELEMENT_ARRAY_BUFFER, cubeLineIndices.size() * sizeof(uint32_t), cubeLineIndices.data(), GL_STATIC_DRAW);

	// Default cube
	GLuint defaultCubeVertexBuffer;
	gl.glGenBuffers(1, &defaultCubeVertexBuffer);
	std::vector<RendererResourceManager::Mesh::Vertex> defaultCubeVertices = { { { 0.05f, 0.05f, -0.05f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f } }, { { -0.05f, 0.05f, -0.05f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f } }, { { -0.05f, 0.05f, 0.05f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f } }, { { 0.05f, 0.05f, 0.05f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } }, { { 0.05f, -0.05f, 0.05f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f } }, { { 0.05f, 0.05f, 0.05f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } }, { { -0.05f, 0.05f, 0.05f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } }, { { -0.05f, -0.05f, 0.05f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } }, { { -0.05f, -0.05f, 0.05f }, { -1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } }, { { -0.05f, 0.05f, 0.05f }, { -1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f } }, { { -0.05f, 0.05f, -0.05f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f } }, { { -0.05f, -0.05f, -0.05f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } }, { { -0.05f, -0.05f, -0.05f }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 0.0f } }, { { 0.05f, -0.05f, -0.05f }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 1.0f } }, { { 0.05f, -0.05f, 0.05f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 1.0f } }, { { -0.05f, -0.05f, 0.05f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f } }, { { 0.05f, -0.05f, -0.05f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } }, { { 0.05f, 0.05f, -0.05f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f } }, { { 0.05f, 0.05f, 0.05f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f } }, { { 0.05f, -0.05f, 0.05f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } }, { { -0.05f, -0.05f, -0.05f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 0.0f } }, { { -0.05f, 0.05f, -0.05f}, { 0.0f, 0.0f, -1.0f }, { 1.0f, 1.0f } }, { { 0.05f, 0.05f, -0.05f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f } }, { { 0.05f, -0.05f, -0.05f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f } } };
	gl.glBindBuffer(GL_ARRAY_BUFFER, defaultCubeVertexBuffer);
	gl.glBufferData(GL_ARRAY_BUFFER, defaultCubeVertices.size() * sizeof(RendererResourceManager::Mesh::Vertex), defaultCubeVertices.data(), GL_STATIC_DRAW);

	RendererPrimitive defaultCubePrimitive;
	defaultCubePrimitive.mesh.vertexBuffer = defaultCubeVertexBuffer;
	defaultCubePrimitive.mesh.indexBuffer = cubeTriangleIndexBuffer;
	defaultCubePrimitive.mesh.indexCount = static_cast<GLuint>(cubeTriangleIndices.size());

	RendererModel defaultCubeModel;
	defaultCubeModel.primitives.push_back(defaultCubePrimitive);
	m_globalInfo.rendererResourceManager.rendererModels["defaultCube"] = defaultCubeModel;

	// Default textures
	GLuint defaultDiffuseTexture;
	gl.glGenTextures(1, &defaultDiffuseTexture);
	std::vector<uint8_t> diffuseTextureData = { 145, 99, 65, 255, 208, 194, 175, 255, 208, 194, 175, 255, 145, 99, 65, 255 };
	gl.glBindTexture(GL_TEXTURE_2D, defaultDiffuseTexture);
	gl.glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, diffuseTextureData.data());
	gl.glGenerateMipmap(GL_TEXTURE_2D);
	m_globalInfo.rendererResourceManager.textures["defaultDiffuseTexture"] = defaultDiffuseTexture;

	GLuint defaultEmissiveTexture;
	gl.glGenTextures(1, &defaultEmissiveTexture);
	std::vector<uint8_t> emissiveTextureData = { 0, 0, 0, 255 };
	gl.glBindTexture(GL_TEXTURE_2D, defaultEmissiveTexture);
	gl.glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, emissiveTextureData.data());
	m_globalInfo.rendererResourceManager.textures["defaultEmissiveTexture"] = defaultEmissiveTexture;

	RendererSampler defaultSampler;
	defaultSampler.minFilter = GL_NEAREST;
	defaultSampler.magFilter = GL_NEAREST;
	defaultSampler.mipmapFilter = GL_NEAREST;
	defaultSampler.wrapS = GL_CLAMP_TO_EDGE;
	defaultSampler.wrapT = GL_CLAMP_TO_EDGE;
	m_globalInfo.rendererResourceManager.samplers["defaultSampler"] = defaultSampler;

	// Frustum cube
	GLuint cameraFrustumCubeVertexBuffer;
	gl.glGenBuffers(1, &cameraFrustumCubeVertexBuffer);
	std::vector<RendererResourceManager::Mesh::Vertex> cameraFrustumCubeVertices = { { { -1.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } }, { { 1.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } }, { { 1.0f, -1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } }, { { -1.0f, -1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } }, { { -1.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } }, { { 1.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } }, { { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } }, { { -1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } } };
	gl.glBindBuffer(GL_ARRAY_BUFFER, cameraFrustumCubeVertexBuffer);
	gl.glBufferData(GL_ARRAY_BUFFER, cameraFrustumCubeVertices.size() * sizeof(RendererResourceManager::Mesh::Vertex), cameraFrustumCubeVertices.data(), GL_STATIC_DRAW);

	RendererPrimitive cameraFrustumCubePrimitive;
	cameraFrustumCubePrimitive.mesh.vertexBuffer = cameraFrustumCubeVertexBuffer;
	cameraFrustumCubePrimitive.mesh.indexBuffer = cubeLineIndexBuffer;
	cameraFrustumCubePrimitive.mesh.indexCount = static_cast<GLuint>(cubeLineIndices.size());

	RendererModel cameraFrustumCubeModel;
	cameraFrustumCubeModel.primitives.push_back(cameraFrustumCubePrimitive);
	m_globalInfo.rendererResourceManager.rendererModels["cameraFrustumCube"] = cameraFrustumCubeModel;

	// Gizmo
	std::array<std::string, 3> gizmoNames = { "Gizmo-Translate", "Gizmo-Rotate", "Gizmo-Scale" };

	m_globalInfo.rendererResourceManager.loadModel("assets/gizmo-translate.obj", gizmoNames[0]);
	m_globalInfo.rendererResourceManager.loadModel("assets/gizmo-rotate.obj", gizmoNames[1]);
	m_globalInfo.rendererResourceManager.loadModel("assets/gizmo-scale.obj", gizmoNames[2]);
	for (const std::string& gizmoName : gizmoNames) {
		if (m_globalInfo.rendererResourceManager.models.find(gizmoName) != m_globalInfo.rendererResourceManager.models.end()) {
			std::array<uint8_t, 3> axisIndices;
			RendererResourceManager::Model& gizmoModel = m_globalInfo.rendererResourceManager.models[gizmoName];
			for (size_t i = 0; i < gizmoModel.primitives.size(); i++) {
				if (gizmoModel.primitives[i].name == "AxisX") {
					axisIndices[0] = static_cast<uint8_t>(i);
				}
				else if (gizmoModel.primitives[i].name == "AxisY") {
					axisIndices[1] = static_cast<uint8_t>(i);
				}
				else if (gizmoModel.primitives[i].name == "AxisZ") {
					axisIndices[2] = static_cast<uint8_t>(i);
				}
			}

			if (axisIndices[0] != 0) {
				std::swap(gizmoModel.primitives[0], gizmoModel.primitives[axisIndices[0]]);
				std::swap(axisIndices[0], axisIndices[axisIndices[0]]);
			}
			if (axisIndices[1] != 1) {
				std::swap(gizmoModel.primitives[1], gizmoModel.primitives[axisIndices[1]]);
				std::swap(axisIndices[1], axisIndices[axisIndices[1]]);
			}
			if (axisIndices[2] != 2) {
				std::swap(gizmoModel.primitives[2], gizmoModel.primitives[axisIndices[2]]);
				std::swap(axisIndices[2], axisIndices[axisIndices[2]]);
			}
		}
	}

	// Light
	createLightBuffer();

	// Start render
	m_waitTimer.setInterval(16);
	m_waitTimer.start();
}

void Renderer::paintGL() {
	if (m_gotResized) {
		destroyPickingImages();
		createPickingImages();
		destroyOutlineSoloImages();
		createOutlineSoloImages();

		m_gotResized = false;
	}

	loadResourcesToGPU();

	updateCamera();

	if (m_globalInfo.editorParameters.renderer.enableLighting) {
		updateLights();
	}

	if (m_globalInfo.editorParameters.renderer.enableBackfaceCulling) {
		gl.glEnable(GL_CULL_FACE);
	}
	else {
		gl.glDisable(GL_CULL_FACE);
	}

	gl.glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
	gl.glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	gl.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gl.glEnable(GL_DEPTH_TEST);
	gl.glDepthFunc(GL_LESS);
	gl.glDepthMask(GL_TRUE);
	gl.glEnable(GL_BLEND);
	gl.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Entities
	gl.glUseProgram(m_entityProgram);
	gl.glUniformMatrix4fv(gl.glGetUniformLocation(m_entityProgram, "viewProj"), 1, false, m_camera.viewProjMatrix.data());

	for (const auto& entity : m_globalInfo.entities) {
		if (entity.second.isVisible) {
			bool hasEntityMoveTransform = m_entityMoveTransforms.find(entity.second.entityID) != m_entityMoveTransforms.end();
			const Transform& transform = hasEntityMoveTransform ? m_entityMoveTransforms[entity.second.entityID] : entity.second.transform;
			nml::mat4 rotationMatrix = nml::rotate(nml::toRad(transform.rotation.x), nml::vec3(1.0f, 0.0f, 0.0f)) * nml::rotate(nml::toRad(transform.rotation.y), nml::vec3(0.0f, 1.0f, 0.0f)) * nml::rotate(nml::toRad(transform.rotation.z), nml::vec3(0.0f, 0.0f, 1.0f));
			nml::mat4 modelMatrix = nml::translate(transform.position) * rotationMatrix * nml::scale(transform.scale);

			gl.glUniformMatrix4fv(gl.glGetUniformLocation(m_entityProgram, "model"), 1, false, modelMatrix.data());

			GLuint indexCount = 0;

			bool hasMesh = false;
			RendererMaterial primitiveMaterial;
			if (entity.second.renderable &&
				(m_globalInfo.rendererResourceManager.rendererModels.find(entity.second.renderable->modelPath) != m_globalInfo.rendererResourceManager.rendererModels.end()) &&
				(entity.second.renderable->primitiveIndex != NTSHENGN_NO_MODEL_PRIMITIVE) &&
				(entity.second.renderable->primitiveIndex < m_globalInfo.rendererResourceManager.rendererModels[entity.second.renderable->modelPath].primitives.size())) {
				// Entity has a mesh
				const RendererModel& entityModel = m_globalInfo.rendererResourceManager.rendererModels[entity.second.renderable->modelPath];
				const RendererPrimitive& entityPrimitive = entityModel.primitives[entity.second.renderable->primitiveIndex];
				const RendererMesh& entityMesh = entityPrimitive.mesh;
				primitiveMaterial = entityPrimitive.material;

				gl.glBindBuffer(GL_ARRAY_BUFFER, entityMesh.vertexBuffer);
				GLint positionPos = gl.glGetAttribLocation(m_entityProgram, "position");
				GLint normalPos = gl.glGetAttribLocation(m_entityProgram, "normal");
				GLint uvPos = gl.glGetAttribLocation(m_entityProgram, "uv");
				gl.glEnableVertexAttribArray(positionPos);
				gl.glVertexAttribPointer(positionPos, 3, GL_FLOAT, false, 32, (void*)0);
				gl.glEnableVertexAttribArray(normalPos);
				gl.glVertexAttribPointer(normalPos, 3, GL_FLOAT, false, 32, (void*)12);
				gl.glEnableVertexAttribArray(uvPos);
				gl.glVertexAttribPointer(uvPos, 2, GL_FLOAT, false, 32, (void*)24);
				gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, entityMesh.indexBuffer);

				indexCount = entityMesh.indexCount;
				hasMesh = true;
			}
			else {
				// Entity does not have a mesh, default cube
				const RendererPrimitive& defaultModelPrimitive = m_globalInfo.rendererResourceManager.rendererModels["defaultCube"].primitives[0];
				const RendererMesh& defaultMesh = defaultModelPrimitive.mesh;
				gl.glBindBuffer(GL_ARRAY_BUFFER, defaultMesh.vertexBuffer);
				GLint positionPos = gl.glGetAttribLocation(m_entityProgram, "position");
				GLint normalPos = gl.glGetAttribLocation(m_entityProgram, "normal");
				GLint uvPos = gl.glGetAttribLocation(m_entityProgram, "uv");
				gl.glEnableVertexAttribArray(positionPos);
				gl.glVertexAttribPointer(positionPos, 3, GL_FLOAT, false, 32, (void*)0);
				gl.glEnableVertexAttribArray(normalPos);
				gl.glVertexAttribPointer(normalPos, 3, GL_FLOAT, false, 32, (void*)12);
				gl.glEnableVertexAttribArray(uvPos);
				gl.glVertexAttribPointer(uvPos, 2, GL_FLOAT, false, 32, (void*)24);
				gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, defaultMesh.indexBuffer);

				indexCount = defaultMesh.indexCount;
			}

			if (hasMesh &&
				entity.second.renderable) {
				// Entity has a material
				const RendererMaterial& material = !entity.second.renderable->materialPath.empty() ? m_globalInfo.rendererResourceManager.materials[entity.second.renderable->materialPath] : primitiveMaterial;

				gl.glActiveTexture(GL_TEXTURE0);
				if (m_globalInfo.rendererResourceManager.textures.find(material.diffuseTextureName) != m_globalInfo.rendererResourceManager.textures.end()) {
					gl.glBindTexture(GL_TEXTURE_2D, m_globalInfo.rendererResourceManager.textures[material.diffuseTextureName]);
				}
				else if (std::filesystem::path(material.diffuseTextureName).is_relative() && (m_globalInfo.rendererResourceManager.textures.find(AssetHelper::relativeToAbsolute(material.diffuseTextureName, m_globalInfo.projectDirectory)) != m_globalInfo.rendererResourceManager.textures.end())) { // Texture may have been registered under another name, its full path
					gl.glBindTexture(GL_TEXTURE_2D, m_globalInfo.rendererResourceManager.textures[AssetHelper::relativeToAbsolute(material.diffuseTextureName, m_globalInfo.projectDirectory)]);
				}
				m_globalInfo.rendererResourceManager.samplers[material.diffuseTextureSamplerName].bind(gl);
				gl.glUniform1i(gl.glGetUniformLocation(m_entityProgram, "diffuseTextureSampler"), 0);

				gl.glActiveTexture(GL_TEXTURE1);
				if (m_globalInfo.rendererResourceManager.textures.find(material.emissiveTextureName) != m_globalInfo.rendererResourceManager.textures.end()) {
					gl.glBindTexture(GL_TEXTURE_2D, m_globalInfo.rendererResourceManager.textures[material.emissiveTextureName]);
				}
				else if (std::filesystem::path(material.emissiveTextureName).is_relative() && (m_globalInfo.rendererResourceManager.textures.find(AssetHelper::relativeToAbsolute(material.emissiveTextureName, m_globalInfo.projectDirectory)) != m_globalInfo.rendererResourceManager.textures.end())) { // Texture may have been registered under another name, its full path
					gl.glBindTexture(GL_TEXTURE_2D, m_globalInfo.rendererResourceManager.textures[AssetHelper::relativeToAbsolute(material.emissiveTextureName, m_globalInfo.projectDirectory)]);
				}
				m_globalInfo.rendererResourceManager.samplers[material.emissiveTextureSamplerName].bind(gl);
				gl.glUniform1i(gl.glGetUniformLocation(m_entityProgram, "emissiveTextureSampler"), 1);

				gl.glUniform1f(gl.glGetUniformLocation(m_entityProgram, "emissiveFactor"), material.emissiveFactor);

				gl.glUniform1f(gl.glGetUniformLocation(m_entityProgram, "alphaCutoff"), material.alphaCutoff);

				gl.glUniform1i(gl.glGetUniformLocation(m_entityProgram, "useTriplanarMapping"), material.useTriplanarMapping);

				gl.glUniform2f(gl.glGetUniformLocation(m_entityProgram, "scaleUV"), material.scaleUV.x, material.scaleUV.y);

				gl.glUniform2f(gl.glGetUniformLocation(m_entityProgram, "offsetUV"), material.offsetUV.x, material.offsetUV.y);

				gl.glUniform1i(gl.glGetUniformLocation(m_entityProgram, "enableShading"), m_globalInfo.editorParameters.renderer.enableLighting);
			}
			else {
				// Entity has no material or no mesh, default material
				RendererPrimitive& defaultModelPrimitive = m_globalInfo.rendererResourceManager.rendererModels["defaultCube"].primitives[0];
				RendererMaterial& defaultMaterial = defaultModelPrimitive.material;

				gl.glActiveTexture(GL_TEXTURE0);
				gl.glBindTexture(GL_TEXTURE_2D, m_globalInfo.rendererResourceManager.textures[defaultMaterial.diffuseTextureName]);
				m_globalInfo.rendererResourceManager.samplers[defaultMaterial.diffuseTextureSamplerName].bind(gl);
				gl.glUniform1i(gl.glGetUniformLocation(m_entityProgram, "diffuseTextureSampler"), 0);

				gl.glActiveTexture(GL_TEXTURE1);
				gl.glBindTexture(GL_TEXTURE_2D, m_globalInfo.rendererResourceManager.textures[defaultMaterial.emissiveTextureName]);
				m_globalInfo.rendererResourceManager.samplers[defaultMaterial.emissiveTextureSamplerName].bind(gl);
				gl.glUniform1i(gl.glGetUniformLocation(m_entityProgram, "emissiveTextureSampler"), 1);

				gl.glUniform1f(gl.glGetUniformLocation(m_entityProgram, "emissiveFactor"), defaultMaterial.emissiveFactor);

				gl.glUniform1f(gl.glGetUniformLocation(m_entityProgram, "alphaCutoff"), defaultMaterial.alphaCutoff);

				gl.glUniform1i(gl.glGetUniformLocation(m_entityProgram, "useTriplanarMapping"), defaultMaterial.useTriplanarMapping);

				gl.glUniform2f(gl.glGetUniformLocation(m_entityProgram, "scaleUV"), defaultMaterial.scaleUV.x, defaultMaterial.scaleUV.y);

				gl.glUniform2f(gl.glGetUniformLocation(m_entityProgram, "offsetUV"), defaultMaterial.offsetUV.x, defaultMaterial.offsetUV.y);

				gl.glUniform1i(gl.glGetUniformLocation(m_entityProgram, "enableShading"), 0);
			}

			gl.glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, NULL);
		}
	}

	gl.glDisable(GL_CULL_FACE);
	
	// Entities Cameras
	if (m_globalInfo.editorParameters.renderer.showCameras) {
		gl.glUseProgram(m_cameraFrustumProgram);
		gl.glUniformMatrix4fv(gl.glGetUniformLocation(m_cameraFrustumProgram, "viewProj"), 1, false, m_camera.viewProjMatrix.data());

		gl.glBindBuffer(GL_ARRAY_BUFFER, m_globalInfo.rendererResourceManager.rendererModels["cameraFrustumCube"].primitives[0].mesh.vertexBuffer);
		GLint positionPos = gl.glGetAttribLocation(m_cameraFrustumProgram, "position");
		gl.glEnableVertexAttribArray(positionPos);
		gl.glVertexAttribPointer(positionPos, 3, GL_FLOAT, false, 32, (void*)0);
		gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_globalInfo.rendererResourceManager.rendererModels["cameraFrustumCube"].primitives[0].mesh.indexBuffer);

		for (const auto& entity : m_globalInfo.entities) {
			if (entity.second.isVisible) {
				if (entity.second.camera) {
					bool hasEntityMoveTransform = m_entityMoveTransforms.find(entity.second.entityID) != m_entityMoveTransforms.end();
					const Transform& transform = hasEntityMoveTransform ? m_entityMoveTransforms[entity.second.entityID] : entity.second.transform;
					nml::mat4 entityCameraViewMatrix = nml::lookAtRH(transform.position, transform.position + entity.second.camera->forward, entity.second.camera->up);
					nml::mat4 entityCameraRotation = nml::rotate(nml::toRad(transform.rotation.x), nml::vec3(1.0f, 0.0f, 0.0f)) * nml::rotate(nml::toRad(transform.rotation.y), nml::vec3(0.0f, 1.0f, 0.0f)) * nml::rotate(nml::toRad(transform.rotation.z), nml::vec3(0.0f, 0.0f, 1.0f));
					nml::mat4 entityCameraProjectionMatrix = nml::mat4::identity();
					float defaultAspectRatio = 16.0f / 9.0f;
					if (entity.second.camera->projectionType == "Perspective") {
						entityCameraProjectionMatrix = nml::perspectiveRH(nml::toRad(entity.second.camera->fov), defaultAspectRatio, entity.second.camera->nearPlane, entity.second.camera->farPlane);
					}
					else if (entity.second.camera->projectionType == "Orthographic") {
						entityCameraProjectionMatrix = nml::orthoRH(entity.second.camera->left * defaultAspectRatio, entity.second.camera->right * defaultAspectRatio, entity.second.camera->bottom, entity.second.camera->top, entity.second.camera->nearPlane, entity.second.camera->farPlane);
					}
					nml::mat4 invEntityCameraModel = nml::inverse(entityCameraProjectionMatrix * entityCameraRotation * entityCameraViewMatrix);
					gl.glUniformMatrix4fv(gl.glGetUniformLocation(m_cameraFrustumProgram, "model"), 1, false, invEntityCameraModel.data());

					gl.glDrawElements(GL_LINES, m_globalInfo.rendererResourceManager.rendererModels["cameraFrustumCube"].primitives[0].mesh.indexCount, GL_UNSIGNED_INT, NULL);
				}
			}
		}
	}

	// Entities Colliders
	if (m_globalInfo.editorParameters.renderer.showColliders) {
		gl.glDepthFunc(GL_LEQUAL);

		gl.glUseProgram(m_colliderProgram);
		gl.glUniformMatrix4fv(gl.glGetUniformLocation(m_colliderProgram, "viewProj"), 1, false, m_camera.viewProjMatrix.data());

		for (const auto& entity : m_globalInfo.entities) {
			if (entity.second.isVisible) {
				if (entity.second.collidable && (m_globalInfo.rendererResourceManager.rendererModels.find("Collider_" + std::to_string(entity.first)) != m_globalInfo.rendererResourceManager.rendererModels.end())) {
					bool hasEntityMoveTransform = m_entityMoveTransforms.find(entity.second.entityID) != m_entityMoveTransforms.end();
					const Transform& transform = hasEntityMoveTransform ? m_entityMoveTransforms[entity.second.entityID] : entity.second.transform;
					nml::mat4 rotationMatrix = nml::rotate(nml::toRad(transform.rotation.x), nml::vec3(1.0f, 0.0f, 0.0f)) * nml::rotate(nml::toRad(transform.rotation.y), nml::vec3(0.0f, 1.0f, 0.0f)) * nml::rotate(nml::toRad(transform.rotation.z), nml::vec3(0.0f, 0.0f, 1.0f));
					nml::mat4 modelMatrix = nml::translate(transform.position) * rotationMatrix * nml::scale(transform.scale);

					gl.glUniformMatrix4fv(gl.glGetUniformLocation(m_colliderProgram, "model"), 1, false, modelMatrix.data());

					if (entity.second.rigidbody) {
						gl.glUniform3f(gl.glGetUniformLocation(m_colliderProgram, "colliderColor"), 1.0f, 0.0f, 0.0f);
					}
					else {
						gl.glUniform3f(gl.glGetUniformLocation(m_colliderProgram, "colliderColor"), 0.5f, 0.0f, 0.0f);
					}

					const RendererModel& colliderModel = m_globalInfo.rendererResourceManager.rendererModels["Collider_" + std::to_string(entity.first)];
					const RendererPrimitive& colliderPrimitive = colliderModel.primitives[0];
					gl.glBindBuffer(GL_ARRAY_BUFFER, colliderPrimitive.mesh.vertexBuffer);
					GLint positionPos = gl.glGetAttribLocation(m_colliderProgram, "position");
					gl.glEnableVertexAttribArray(positionPos);
					gl.glVertexAttribPointer(positionPos, 3, GL_FLOAT, false, 32, (void*)0);
					gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, colliderPrimitive.mesh.indexBuffer);

					gl.glDrawElements(GL_LINES, colliderPrimitive.mesh.indexCount, GL_UNSIGNED_INT, NULL);
				}
			}
		}
	}

	// Grid
	if (m_globalInfo.editorParameters.renderer.showGrid) {
		if (!m_camera.useOrthographicProjection) {
			gl.glDepthFunc(GL_LESS);

			gl.glUseProgram(m_gridProgram);
			gl.glUniformMatrix4fv(gl.glGetUniformLocation(m_gridProgram, "view"), 1, false, m_camera.viewMatrix.data());
			gl.glUniformMatrix4fv(gl.glGetUniformLocation(m_gridProgram, "projection"), 1, false, m_camera.projectionMatrix.data());
			gl.glUniformMatrix4fv(gl.glGetUniformLocation(m_gridProgram, "viewProj"), 1, false, m_camera.viewProjMatrix.data());
			gl.glUniform1f(gl.glGetUniformLocation(m_gridProgram, "near"), m_globalInfo.editorParameters.renderer.cameraNearPlane);
			gl.glUniform1f(gl.glGetUniformLocation(m_gridProgram, "far"), m_globalInfo.editorParameters.renderer.cameraFarPlane);
			gl.glUniform1f(gl.glGetUniformLocation(m_gridProgram, "gridScale"), m_globalInfo.editorParameters.renderer.gridScale);

			gl.glDrawArrays(GL_TRIANGLES, 0, 6);
		}
	}

	if (m_globalInfo.editorParameters.renderer.enableBackfaceCulling) {
		gl.glEnable(GL_CULL_FACE);
	}
	else {
		gl.glDisable(GL_CULL_FACE);
	}

	nml::vec3 gizmoPosition = nml::vec3(0.0f, 0.0f, 0.0f);
	bool gizmoPositionCalculated = false;
	// Picking
	if (m_doPicking) {
		gl.glBindFramebuffer(GL_FRAMEBUFFER, m_pickingFramebuffer);
		GLenum bufferEnum = GL_COLOR_ATTACHMENT0;
		glex.glDrawBuffers(1, &bufferEnum);
		GLuint maxUint = NO_ENTITY;
		glex.glClearBufferuiv(GL_COLOR, 0, &maxUint);
		gl.glClear(GL_DEPTH_BUFFER_BIT);
		gl.glEnable(GL_DEPTH_TEST);
		gl.glDepthFunc(GL_LESS);
		gl.glDepthMask(GL_TRUE);
		gl.glDisable(GL_BLEND);

		gl.glUseProgram(m_pickingProgram);
		gl.glUniformMatrix4fv(gl.glGetUniformLocation(m_pickingProgram, "viewProj"), 1, false, m_camera.viewProjMatrix.data());

		for (const auto& entity : m_globalInfo.entities) {
			if (entity.second.isVisible) {
				bool hasEntityMoveTransform = m_entityMoveTransforms.find(entity.second.entityID) != m_entityMoveTransforms.end();
				const Transform& transform = hasEntityMoveTransform ? m_entityMoveTransforms[entity.second.entityID] : entity.second.transform;
				nml::mat4 rotationMatrix = nml::rotate(nml::toRad(transform.rotation.x), nml::vec3(1.0f, 0.0f, 0.0f)) * nml::rotate(nml::toRad(transform.rotation.y), nml::vec3(0.0f, 1.0f, 0.0f)) * nml::rotate(nml::toRad(transform.rotation.z), nml::vec3(0.0f, 0.0f, 1.0f));
				nml::mat4 modelMatrix = nml::translate(transform.position) * rotationMatrix * nml::scale(transform.scale);
				gl.glUniformMatrix4fv(gl.glGetUniformLocation(m_pickingProgram, "model"), 1, false, modelMatrix.data());

				glex.glUniform1ui(gl.glGetUniformLocation(m_pickingProgram, "entityID"), entity.second.entityID);

				if (entity.second.renderable && (m_globalInfo.rendererResourceManager.rendererModels.find(entity.second.renderable->modelPath) != m_globalInfo.rendererResourceManager.rendererModels.end())) {
					const RendererModel& entityModel = m_globalInfo.rendererResourceManager.rendererModels[entity.second.renderable->modelPath];
					if ((entity.second.renderable->primitiveIndex != NTSHENGN_NO_MODEL_PRIMITIVE) && (entity.second.renderable->primitiveIndex < entityModel.primitives.size())) {
						const RendererPrimitive& entityPrimitive = entityModel.primitives[entity.second.renderable->primitiveIndex];
						gl.glBindBuffer(GL_ARRAY_BUFFER, entityPrimitive.mesh.vertexBuffer);
						GLint positionPos = gl.glGetAttribLocation(m_pickingProgram, "position");
						gl.glEnableVertexAttribArray(positionPos);
						gl.glVertexAttribPointer(positionPos, 3, GL_FLOAT, false, 32, (void*)0);
						gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, entityPrimitive.mesh.indexBuffer);

						gl.glDrawElements(GL_TRIANGLES, entityPrimitive.mesh.indexCount, GL_UNSIGNED_INT, NULL);
					}
					else {
						RendererMesh& defaultMesh = m_globalInfo.rendererResourceManager.rendererModels["defaultCube"].primitives[0].mesh;
						gl.glBindBuffer(GL_ARRAY_BUFFER, defaultMesh.vertexBuffer);
						GLint positionPos = gl.glGetAttribLocation(m_pickingProgram, "position");
						gl.glEnableVertexAttribArray(positionPos);
						gl.glVertexAttribPointer(positionPos, 3, GL_FLOAT, false, 32, (void*)0);
						gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, defaultMesh.indexBuffer);

						gl.glDrawElements(GL_TRIANGLES, defaultMesh.indexCount, GL_UNSIGNED_INT, NULL);
					}
				}
				else {
					RendererMesh& defaultMesh = m_globalInfo.rendererResourceManager.rendererModels["defaultCube"].primitives[0].mesh;
					gl.glBindBuffer(GL_ARRAY_BUFFER, defaultMesh.vertexBuffer);
					GLint positionPos = gl.glGetAttribLocation(m_pickingProgram, "position");
					gl.glEnableVertexAttribArray(positionPos);
					gl.glVertexAttribPointer(positionPos, 3, GL_FLOAT, false, 32, (void*)0);
					gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, defaultMesh.indexBuffer);

					gl.glDrawElements(GL_TRIANGLES, defaultMesh.indexCount, GL_UNSIGNED_INT, NULL);
				}
			}
		}

		if (m_globalInfo.currentEntityID != NO_ENTITY) {
			bool hasEntityMoveTransform = m_entityMoveTransforms.find(m_globalInfo.currentEntityID) != m_entityMoveTransforms.end();
			gizmoPosition = hasEntityMoveTransform ? m_entityMoveTransforms[m_globalInfo.currentEntityID].position : m_globalInfo.entities[m_globalInfo.currentEntityID].transform.position;
			for (EntityID otherSelectedEntityID : m_globalInfo.otherSelectedEntityIDs) {
				hasEntityMoveTransform = m_entityMoveTransforms.find(otherSelectedEntityID) != m_entityMoveTransforms.end();
				gizmoPosition += hasEntityMoveTransform ? m_entityMoveTransforms[otherSelectedEntityID].position : m_globalInfo.entities[otherSelectedEntityID].transform.position;
			}
			gizmoPosition /= static_cast<float>(m_globalInfo.otherSelectedEntityIDs.size() + 1);

			gizmoPositionCalculated = true;

			std::string gizmoModelName = "";
			if (m_gizmoMode == GizmoMode::Translate) {
				gizmoModelName = "Gizmo-Translate";
			}
			else if (m_gizmoMode == GizmoMode::Rotate) {
				gizmoModelName = "Gizmo-Rotate";
			}
			else if (m_gizmoMode == GizmoMode::Scale) {
				gizmoModelName = "Gizmo-Scale";
			}
			else if (m_gizmoMode == GizmoMode::None) {
				gizmoModelName = "";
			}
			if (m_gizmoMode != GizmoMode::None) {
				if (m_globalInfo.rendererResourceManager.rendererModels.find(gizmoModelName) != m_globalInfo.rendererResourceManager.rendererModels.end()) {
					gl.glClear(GL_DEPTH_BUFFER_BIT);
					gl.glEnable(GL_CULL_FACE);

					float gizmoScaling = 1.0f;
					if (m_globalInfo.editorParameters.renderer.maintainGizmoSize) {
						if (m_camera.useOrthographicProjection) {
							gizmoScaling = m_camera.orthographicHalfExtent;
						}
						else {
							gizmoScaling = (m_camera.position - gizmoPosition).length() / 4.0f;
						}
					}
					nml::mat4 scaleMatrix = nml::scale(nml::vec3(m_globalInfo.editorParameters.renderer.gizmoSize * gizmoScaling));
					nml::mat4 modelMatrix = nml::translate(gizmoPosition) * scaleMatrix;
					gl.glUniformMatrix4fv(gl.glGetUniformLocation(m_pickingProgram, "model"), 1, false, modelMatrix.data());

					const RendererModel& gizmoModel = m_globalInfo.rendererResourceManager.rendererModels[gizmoModelName];
					for (size_t i = 0; i < gizmoModel.primitives.size(); i++) {
						glex.glUniform1ui(gl.glGetUniformLocation(m_pickingProgram, "entityID"), NO_ENTITY - (3 - static_cast<GLuint>(i)));

						const RendererPrimitive& gizmoPrimitive = gizmoModel.primitives[i];
						gl.glBindBuffer(GL_ARRAY_BUFFER, gizmoPrimitive.mesh.vertexBuffer);
						GLint positionPos = gl.glGetAttribLocation(m_pickingProgram, "position");
						gl.glEnableVertexAttribArray(positionPos);
						gl.glVertexAttribPointer(positionPos, 3, GL_FLOAT, false, 32, (void*)0);
						gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gizmoPrimitive.mesh.indexBuffer);

						gl.glDrawElements(GL_TRIANGLES, gizmoPrimitive.mesh.indexCount, GL_UNSIGNED_INT, NULL);
					}
				}
			}
		}

		QPoint cursorPosition = mapFromGlobal(QCursor::pos());
		uint32_t pickedEntityID;
		gl.glReadPixels(cursorPosition.x() * m_globalInfo.devicePixelRatio, ((height() - 1) - cursorPosition.y()) * m_globalInfo.devicePixelRatio, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, &pickedEntityID);
		if (pickedEntityID != NO_ENTITY) {
			EntityID currentEntityID = m_globalInfo.currentEntityID;
			std::set<EntityID> otherSelectedEntityIDs = m_globalInfo.otherSelectedEntityIDs;
			if (pickedEntityID < (NO_ENTITY - 3)) {
				if (m_globalInfo.currentEntityID != pickedEntityID) {
					if (m_multiSelectionKeyPressed) {
						if (m_globalInfo.currentEntityID != NO_ENTITY) {
							otherSelectedEntityIDs.insert(m_globalInfo.currentEntityID);
						}
						otherSelectedEntityIDs.erase(pickedEntityID);
						currentEntityID = pickedEntityID;
					}
					else if (QGuiApplication::keyboardModifiers() == Qt::ControlModifier) {
						otherSelectedEntityIDs.erase(pickedEntityID);
					}
					else {
						otherSelectedEntityIDs.clear();
						currentEntityID = pickedEntityID;
					}
				}
				else {
					if (!m_multiSelectionKeyPressed) {
						otherSelectedEntityIDs.clear();
						currentEntityID = pickedEntityID;
					}
				}
				if (m_dragDropResourceType == DragDropResourceType::None) {
					m_globalInfo.selectionUndoStack->push(new SelectAssetEntitiesCommand(m_globalInfo, SelectionType::Entities, "", currentEntityID, otherSelectedEntityIDs));
				}
			}
			else {
				if (pickedEntityID == (NO_ENTITY - 3)) {
					m_gizmoAxis = GizmoAxis::X;
				}
				else if (pickedEntityID == (NO_ENTITY - 2)) {
					m_gizmoAxis = GizmoAxis::Y;
				}
				else if (pickedEntityID == (NO_ENTITY - 1)) {
					m_gizmoAxis = GizmoAxis::Z;
				}
				startTransform();
			}
		}
		else {
			m_globalInfo.selectionUndoStack->push(new SelectAssetEntitiesCommand(m_globalInfo, SelectionType::Entities, "", NO_ENTITY, {}));
		}

		if (m_dragDropResourceType != DragDropResourceType::None) {
			if (m_dragDropResourceType == DragDropResourceType::Model) {
				m_globalInfo.rendererResourceManager.loadModel(AssetHelper::relativeToAbsolute(m_dragDropResourcePath, m_globalInfo.projectDirectory), m_dragDropResourcePath);
				if (pickedEntityID < (NO_ENTITY - 3)) {
					Entity& pickedEntity = m_globalInfo.entities[pickedEntityID];
					if (pickedEntity.renderable) {
						Renderable newRenderable = pickedEntity.renderable.value();
						newRenderable.modelPath = m_dragDropResourcePath;
						newRenderable.primitiveIndex = 0;
						if (m_globalInfo.rendererResourceManager.models.find(m_dragDropResourcePath) != m_globalInfo.rendererResourceManager.models.end()) {
							if (m_globalInfo.rendererResourceManager.models[m_dragDropResourcePath].primitives.empty()) {
								newRenderable.primitiveIndex = NTSHENGN_NO_MODEL_PRIMITIVE;
							}
						}
						m_globalInfo.selectionUndoStack->push(new ChangeEntitiesComponentCommand(m_globalInfo, { pickedEntityID }, "Renderable", { &newRenderable }));
					}
				}
				else {
					if (m_globalInfo.rendererResourceManager.models.find(m_dragDropResourcePath) != m_globalInfo.rendererResourceManager.models.end()) {
						std::string name = m_dragDropResourcePath;
						size_t lastSlashPosition = name.find_last_of('/');
						if (lastSlashPosition != std::string::npos) {
							name = name.substr(lastSlashPosition + 1);
						}
						size_t dotPosition = name.find_last_of('.');
						if (dotPosition != std::string::npos) {
							name = name.substr(0, dotPosition);
						}
						m_globalInfo.actionUndoStack->push(new CreateEntitiesFromModelCommand(m_globalInfo, name, m_dragDropResourcePath));
					}
				}
			}
			else if (m_dragDropResourceType == DragDropResourceType::Material) {
				if (pickedEntityID < (NO_ENTITY - 3)) {
					Entity& pickedEntity = m_globalInfo.entities[pickedEntityID];
					if (pickedEntity.renderable) {
						Renderable newRenderable = pickedEntity.renderable.value();
						newRenderable.materialPath = m_dragDropResourcePath;
						m_globalInfo.rendererResourceManager.loadMaterial(AssetHelper::relativeToAbsolute(m_dragDropResourcePath, m_globalInfo.projectDirectory), m_dragDropResourcePath);
						m_globalInfo.selectionUndoStack->push(new ChangeEntitiesComponentCommand(m_globalInfo, { pickedEntityID }, "Renderable", { &newRenderable }));
					}
				}
			}
		}

		m_doPicking = false;
	}

	gl.glDisable(GL_CULL_FACE);

	// Outline
	if (m_globalInfo.currentEntityID != NO_ENTITY) {
		std::set<EntityID> entitiesOutline = m_globalInfo.otherSelectedEntityIDs;
		entitiesOutline.insert(m_globalInfo.currentEntityID);

		for (EntityID entityID : entitiesOutline) {
			const Entity& entity = m_globalInfo.entities[entityID];

			// Outline Solo
			gl.glBindFramebuffer(GL_FRAMEBUFFER, m_outlineSoloFramebuffer);
			gl.glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			gl.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			gl.glEnable(GL_DEPTH_TEST);
			gl.glDepthFunc(GL_LESS);
			gl.glDepthMask(GL_TRUE);
			gl.glDisable(GL_BLEND);

			gl.glUseProgram(m_outlineSoloProgram);
			gl.glUniformMatrix4fv(gl.glGetUniformLocation(m_outlineSoloProgram, "viewProj"), 1, false, m_camera.viewProjMatrix.data());

			// Entity
			bool hasEntityMoveTransform = m_entityMoveTransforms.find(entityID) != m_entityMoveTransforms.end();
			const Transform& transform = hasEntityMoveTransform ? m_entityMoveTransforms[entityID] : entity.transform;
			nml::mat4 rotationMatrix = nml::rotate(nml::toRad(transform.rotation.x), nml::vec3(1.0f, 0.0f, 0.0f)) * nml::rotate(nml::toRad(transform.rotation.y), nml::vec3(0.0f, 1.0f, 0.0f)) * nml::rotate(nml::toRad(transform.rotation.z), nml::vec3(0.0f, 0.0f, 1.0f));
			nml::mat4 modelMatrix = nml::translate(transform.position) * rotationMatrix * nml::scale(transform.scale);
			gl.glUniformMatrix4fv(gl.glGetUniformLocation(m_outlineSoloProgram, "model"), 1, false, modelMatrix.data());

			if (entity.renderable && (m_globalInfo.rendererResourceManager.rendererModels.find(entity.renderable->modelPath) != m_globalInfo.rendererResourceManager.rendererModels.end())) {
				const RendererModel& entityModel = m_globalInfo.rendererResourceManager.rendererModels[entity.renderable->modelPath];
				if ((entity.renderable->primitiveIndex != NTSHENGN_NO_MODEL_PRIMITIVE) && (entity.renderable->primitiveIndex < entityModel.primitives.size())) {
					const RendererPrimitive& entityPrimitive = entityModel.primitives[entity.renderable->primitiveIndex];
					gl.glBindBuffer(GL_ARRAY_BUFFER, entityPrimitive.mesh.vertexBuffer);
					GLint positionPos = gl.glGetAttribLocation(m_outlineSoloProgram, "position");
					gl.glEnableVertexAttribArray(positionPos);
					gl.glVertexAttribPointer(positionPos, 3, GL_FLOAT, false, 32, (void*)0);
					gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, entityPrimitive.mesh.indexBuffer);

					gl.glDrawElements(GL_TRIANGLES, entityPrimitive.mesh.indexCount, GL_UNSIGNED_INT, NULL);
				}
				else {
					RendererMesh& defaultMesh = m_globalInfo.rendererResourceManager.rendererModels["defaultCube"].primitives[0].mesh;
					gl.glBindBuffer(GL_ARRAY_BUFFER, defaultMesh.vertexBuffer);
					GLint positionPos = gl.glGetAttribLocation(m_outlineSoloProgram, "position");
					gl.glEnableVertexAttribArray(positionPos);
					gl.glVertexAttribPointer(positionPos, 3, GL_FLOAT, false, 32, (void*)0);
					gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, defaultMesh.indexBuffer);

					gl.glDrawElements(GL_TRIANGLES, defaultMesh.indexCount, GL_UNSIGNED_INT, NULL);
				}
			}
			else {
				RendererMesh& defaultMesh = m_globalInfo.rendererResourceManager.rendererModels["defaultCube"].primitives[0].mesh;
				gl.glBindBuffer(GL_ARRAY_BUFFER, defaultMesh.vertexBuffer);
				GLint positionPos = gl.glGetAttribLocation(m_outlineSoloProgram, "position");
				gl.glEnableVertexAttribArray(positionPos);
				gl.glVertexAttribPointer(positionPos, 3, GL_FLOAT, false, 32, (void*)0);
				gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, defaultMesh.indexBuffer);

				gl.glDrawElements(GL_TRIANGLES, defaultMesh.indexCount, GL_UNSIGNED_INT, NULL);
			}

			// Entity Camera
			if (m_globalInfo.editorParameters.renderer.showCameras) {
				if (entity.camera) {
					nml::mat4 entityCameraViewMatrix = nml::lookAtRH(transform.position, transform.position + entity.camera->forward, entity.camera->up);
					nml::mat4 entityCameraRotation = nml::rotate(nml::toRad(transform.rotation.x), nml::vec3(1.0f, 0.0f, 0.0f)) * nml::rotate(nml::toRad(transform.rotation.y), nml::vec3(0.0f, 1.0f, 0.0f)) * nml::rotate(nml::toRad(transform.rotation.z), nml::vec3(0.0f, 0.0f, 1.0f));
					nml::mat4 entityCameraProjectionMatrix = nml::mat4::identity();
					float defaultAspectRatio = 16.0f / 9.0f;
					if (entity.camera->projectionType == "Perspective") {
						entityCameraProjectionMatrix = nml::perspectiveRH(nml::toRad(entity.camera->fov), defaultAspectRatio, entity.camera->nearPlane, entity.camera->farPlane);
					}
					else if (entity.camera->projectionType == "Orthographic") {
						entityCameraProjectionMatrix = nml::orthoRH(entity.camera->left * defaultAspectRatio, entity.camera->right * defaultAspectRatio, entity.camera->bottom, entity.camera->top, entity.camera->nearPlane, entity.camera->farPlane);
					}
					nml::mat4 invEntityCameraModel = nml::inverse(entityCameraProjectionMatrix * entityCameraRotation * entityCameraViewMatrix);
					gl.glUniformMatrix4fv(gl.glGetUniformLocation(m_outlineSoloProgram, "model"), 1, false, invEntityCameraModel.data());

					gl.glBindBuffer(GL_ARRAY_BUFFER, m_globalInfo.rendererResourceManager.rendererModels["cameraFrustumCube"].primitives[0].mesh.vertexBuffer);
					GLint positionPos = gl.glGetAttribLocation(m_outlineSoloProgram, "position");
					gl.glEnableVertexAttribArray(positionPos);
					gl.glVertexAttribPointer(positionPos, 3, GL_FLOAT, false, 32, (void*)0);
					gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_globalInfo.rendererResourceManager.rendererModels["cameraFrustumCube"].primitives[0].mesh.indexBuffer);

					gl.glDrawElements(GL_LINES, m_globalInfo.rendererResourceManager.rendererModels["cameraFrustumCube"].primitives[0].mesh.indexCount, GL_UNSIGNED_INT, NULL);
				}
			}

			// Entity Collider
			if (m_globalInfo.editorParameters.renderer.showColliders) {
				if (entity.isVisible) {
					if (entity.collidable && (m_globalInfo.rendererResourceManager.rendererModels.find("Collider_" + std::to_string(entity.entityID)) != m_globalInfo.rendererResourceManager.rendererModels.end())) {
						const RendererModel& colliderModel = m_globalInfo.rendererResourceManager.rendererModels["Collider_" + std::to_string(entity.entityID)];
						gl.glUniformMatrix4fv(gl.glGetUniformLocation(m_outlineSoloProgram, "model"), 1, false, modelMatrix.data());
						const RendererPrimitive& colliderPrimitive = colliderModel.primitives[0];
						gl.glBindBuffer(GL_ARRAY_BUFFER, colliderPrimitive.mesh.vertexBuffer);
						GLint positionPos = gl.glGetAttribLocation(m_outlineSoloProgram, "position");
						gl.glEnableVertexAttribArray(positionPos);
						gl.glVertexAttribPointer(positionPos, 3, GL_FLOAT, false, 32, (void*)0);
						gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, colliderPrimitive.mesh.indexBuffer);

						gl.glDrawElements(GL_LINES, colliderPrimitive.mesh.indexCount, GL_UNSIGNED_INT, NULL);
					}
				}
			}

			// Outline
			gl.glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
			gl.glEnable(GL_DEPTH_TEST);
			gl.glDepthFunc(GL_ALWAYS);

			gl.glUseProgram(m_outlineProgram);
			gl.glActiveTexture(GL_TEXTURE0);
			gl.glBindTexture(GL_TEXTURE_2D, m_outlineSoloImage);
			gl.glUniform1i(gl.glGetUniformLocation(m_outlineProgram, "outlineSoloTexture"), 0);
			if (entity.entityID == m_globalInfo.currentEntityID) {
				gl.glUniform3f(gl.glGetUniformLocation(m_outlineProgram, "outlineColor"), m_globalInfo.editorParameters.renderer.currentEntityOutlineColor.x, m_globalInfo.editorParameters.renderer.currentEntityOutlineColor.y, m_globalInfo.editorParameters.renderer.currentEntityOutlineColor.z);
			}
			else {
				gl.glUniform3f(gl.glGetUniformLocation(m_outlineProgram, "outlineColor"), m_globalInfo.editorParameters.renderer.otherEntitiesOutlineColor.x, m_globalInfo.editorParameters.renderer.otherEntitiesOutlineColor.y, m_globalInfo.editorParameters.renderer.otherEntitiesOutlineColor.z);
			}
			gl.glBindBuffer(GL_ARRAY_BUFFER, 0);

			gl.glDrawArrays(GL_TRIANGLES, 0, 3);
		}
	}

	// Gizmo
	if (m_globalInfo.currentEntityID != NO_ENTITY) {
		if (!gizmoPositionCalculated) {
			bool hasEntityMoveTransform = m_entityMoveTransforms.find(m_globalInfo.currentEntityID) != m_entityMoveTransforms.end();
			gizmoPosition = hasEntityMoveTransform ? m_entityMoveTransforms[m_globalInfo.currentEntityID].position : m_globalInfo.entities[m_globalInfo.currentEntityID].transform.position;
			for (EntityID otherSelectedEntityID : m_globalInfo.otherSelectedEntityIDs) {
				hasEntityMoveTransform = m_entityMoveTransforms.find(otherSelectedEntityID) != m_entityMoveTransforms.end();
				gizmoPosition += hasEntityMoveTransform ? m_entityMoveTransforms[otherSelectedEntityID].position : m_globalInfo.entities[otherSelectedEntityID].transform.position;
			}
			gizmoPosition /= static_cast<float>(m_globalInfo.otherSelectedEntityIDs.size() + 1);

			gizmoPositionCalculated = true;
		}

		std::string gizmoModelName = "";
		if (m_gizmoMode == GizmoMode::Translate) {
			gizmoModelName = "Gizmo-Translate";
		}
		else if (m_gizmoMode == GizmoMode::Rotate) {
			gizmoModelName = "Gizmo-Rotate";
		}
		else if (m_gizmoMode == GizmoMode::Scale) {
			gizmoModelName = "Gizmo-Scale";
		}
		else if (m_gizmoMode == GizmoMode::None) {
			gizmoModelName = "";
		}
		if (m_gizmoMode != GizmoMode::None) {
			if (m_globalInfo.rendererResourceManager.rendererModels.find(gizmoModelName) != m_globalInfo.rendererResourceManager.rendererModels.end()) {
				gl.glUseProgram(m_gizmoProgram);
				gl.glUniformMatrix4fv(gl.glGetUniformLocation(m_gizmoProgram, "viewProj"), 1, false, m_camera.viewProjMatrix.data());

				gl.glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
				gl.glClear(GL_DEPTH_BUFFER_BIT);
				gl.glEnable(GL_DEPTH_TEST);
				gl.glDepthFunc(GL_LESS);
				gl.glDepthMask(GL_TRUE);
				gl.glEnable(GL_CULL_FACE);

				float gizmoScaling = 1.0f;
				if (m_globalInfo.editorParameters.renderer.maintainGizmoSize) {
					if (m_camera.useOrthographicProjection) {
						gizmoScaling = m_camera.orthographicHalfExtent;
					}
					else {
						gizmoScaling = (m_camera.position - gizmoPosition).length() / 4.0f;
					}
				}
				nml::mat4 scaleMatrix = nml::scale(nml::vec3(m_globalInfo.editorParameters.renderer.gizmoSize * gizmoScaling));
				nml::mat4 modelMatrix = nml::translate(gizmoPosition) * scaleMatrix;
				gl.glUniformMatrix4fv(gl.glGetUniformLocation(m_gizmoProgram, "model"), 1, false, modelMatrix.data());

				const RendererModel& gizmoModel = m_globalInfo.rendererResourceManager.rendererModels[gizmoModelName];
				for (size_t i = 0; i < gizmoModel.primitives.size(); i++) {
					const RendererPrimitive& gizmoPrimitive = gizmoModel.primitives[i];
					nml::vec3 gizmoAxisColor = nml::vec3(0.0f, 0.0f, 0.0f);
					if (i == 0) {
						if (m_gizmoAxis == GizmoAxis::X) {
							gizmoAxisColor.x = 1.0f;
						}
						else {
							gizmoAxisColor.x = 0.5f;
						}
					}
					else if (i == 1) {
						if (m_gizmoAxis == GizmoAxis::Y) {
							gizmoAxisColor.y = 1.0f;
						}
						else {
							gizmoAxisColor.y = 0.5f;
						}
					}
					else if (i == 2) {
						if (m_gizmoAxis == GizmoAxis::Z) {
							gizmoAxisColor.z = 1.0f;
						}
						else {
							gizmoAxisColor.z = 0.5f;
						}
					}
					gl.glUniform3f(gl.glGetUniformLocation(m_gizmoProgram, "axisColor"), gizmoAxisColor.x, gizmoAxisColor.y, gizmoAxisColor.z);
					gl.glBindBuffer(GL_ARRAY_BUFFER, gizmoPrimitive.mesh.vertexBuffer);
					GLint positionPos = gl.glGetAttribLocation(m_gizmoProgram, "position");
					gl.glEnableVertexAttribArray(positionPos);
					gl.glVertexAttribPointer(positionPos, 3, GL_FLOAT, false, 32, (void*)0);
					gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gizmoPrimitive.mesh.indexBuffer);

					gl.glDrawElements(GL_TRIANGLES, gizmoPrimitive.mesh.indexCount, GL_UNSIGNED_INT, NULL);
				}
			}
		}
	}

	gl.glDisable(GL_CULL_FACE);

	m_dragDropResourceType = DragDropResourceType::None;
	m_dragDropResourcePath.clear();
}

GLuint Renderer::compileShader(GLenum shaderType, const std::string& shaderCode) {
	GLuint shader = gl.glCreateShader(shaderType);
	const char* shaderCodec_str = shaderCode.c_str();
	GLint shaderLength = static_cast<GLint>(shaderCode.size());
	gl.glShaderSource(shader, 1, &shaderCodec_str, &shaderLength);
	gl.glCompileShader(shader);
	GLint params;
	gl.glGetShaderiv(shader, GL_COMPILE_STATUS, &params);
	if (params != GL_TRUE) {
		GLsizei maxlength;
		gl.glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxlength);
		std::vector<GLchar> infolog(maxlength);
		int length;
		gl.glGetShaderInfoLog(shader, maxlength, &length, infolog.data());
		m_globalInfo.logger.addLog(LogLevel::Error, m_globalInfo.localization.getString("log_shader_compilation_error", { std::string(infolog.data(), length) }));

		return 0xFFFFFFFF;
	}

	return shader;
}

GLuint Renderer::compileProgram(GLuint vertexShader, GLuint fragmentShader) {
	GLuint program = gl.glCreateProgram();
	gl.glAttachShader(program, vertexShader);
	gl.glAttachShader(program, fragmentShader);
	gl.glLinkProgram(program);
	GLint params;
	gl.glGetProgramiv(program, GL_LINK_STATUS, &params);
	if (params != GL_TRUE) {
		GLsizei maxlength;
		gl.glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxlength);
		std::vector<GLchar> infolog(maxlength);
		int length;
		gl.glGetProgramInfoLog(program, maxlength, &length, infolog.data());
		m_globalInfo.logger.addLog(LogLevel::Error, m_globalInfo.localization.getString("log_program_linking_error", { std::string(infolog.data(), length) }));
		gl.glDetachShader(program, vertexShader);
		gl.glDetachShader(program, fragmentShader);

		return 0xFFFFFFFF;
	}

	return program;
}

void Renderer::createPickingImages() {
	gl.glBindFramebuffer(GL_FRAMEBUFFER, m_pickingFramebuffer);

	gl.glGenTextures(1, &m_pickingImage);
	gl.glBindTexture(GL_TEXTURE_2D, m_pickingImage);
	gl.glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, static_cast<GLsizei>(width() * m_globalInfo.devicePixelRatio), static_cast<GLsizei>(height() * m_globalInfo.devicePixelRatio), 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
	gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	gl.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_pickingImage, 0);

	gl.glGenRenderbuffers(1, &m_pickingDepthImage);
	gl.glBindRenderbuffer(GL_RENDERBUFFER, m_pickingDepthImage);
	gl.glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, static_cast<GLsizei>(width() * m_globalInfo.devicePixelRatio), static_cast<GLsizei>(height() * m_globalInfo.devicePixelRatio));
	gl.glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_pickingDepthImage);
}

void Renderer::destroyPickingImages() {
	gl.glDeleteTextures(1, &m_pickingImage);
	gl.glDeleteRenderbuffers(1, &m_pickingDepthImage);
}

void Renderer::createOutlineSoloImages() {
	gl.glBindFramebuffer(GL_FRAMEBUFFER, m_outlineSoloFramebuffer);

	gl.glGenTextures(1, &m_outlineSoloImage);
	gl.glBindTexture(GL_TEXTURE_2D, m_outlineSoloImage);
	gl.glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, static_cast<GLsizei>(width() * m_globalInfo.devicePixelRatio), static_cast<GLsizei>(height() * m_globalInfo.devicePixelRatio), 0, GL_RED, GL_FLOAT, NULL);
	gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	gl.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_outlineSoloImage, 0);

	gl.glGenRenderbuffers(1, &m_outlineSoloDepthImage);
	gl.glBindRenderbuffer(GL_RENDERBUFFER, m_outlineSoloDepthImage);
	gl.glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, static_cast<GLsizei>(width() * m_globalInfo.devicePixelRatio), static_cast<GLsizei>(height() * m_globalInfo.devicePixelRatio));
	gl.glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_outlineSoloDepthImage);
}

void Renderer::destroyOutlineSoloImages() {
	gl.glDeleteTextures(1, &m_outlineSoloImage);
	gl.glDeleteRenderbuffers(1, &m_outlineSoloDepthImage);
}

void Renderer::createLightBuffer() {
	gl.glGenBuffers(1, &m_lightBuffer);
	gl.glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_lightBuffer);
	gl.glBufferData(GL_SHADER_STORAGE_BUFFER, 32768, NULL, GL_DYNAMIC_DRAW);
}

bool Renderer::anyEntityTransformMode() {
	return m_translateEntityMode || m_rotateEntityMode || m_scaleEntityMode;
}

void Renderer::updateCamera() {
	if (anyEntityTransformMode()) {
		return;
	}

	float deltaTime = static_cast<float>(m_waitTimer.interval()) / 1000.0f;

	if (!m_camera.useOrthographicProjection) {
		m_camera.perspectiveYaw = std::fmod(m_camera.perspectiveYaw + (m_mouseCursorDifference.x * m_globalInfo.editorParameters.renderer.cameraSensitivity), 360.0f);
		m_camera.perspectivePitch = std::max(-89.0f, std::min(89.0f, m_camera.perspectivePitch + (m_mouseCursorDifference.y * m_globalInfo.editorParameters.renderer.cameraSensitivity)));

		float cameraYawRad = nml::toRad(m_camera.perspectiveYaw);
		float cameraPitchRad = nml::toRad(m_camera.perspectivePitch);

		m_camera.perspectiveDirection.x = std::cos(cameraPitchRad) * std::cos(cameraYawRad);
		m_camera.perspectiveDirection.y = -std::sin(cameraPitchRad);
		m_camera.perspectiveDirection.z = std::cos(cameraPitchRad) * std::sin(cameraYawRad);
		m_camera.perspectiveDirection = nml::normalize(m_camera.perspectiveDirection);

		nml::vec3 t = nml::normalize(nml::vec3(-m_camera.perspectiveDirection.z, 0.0f, m_camera.perspectiveDirection.x));

		float cameraSpeed = m_globalInfo.editorParameters.renderer.perspectiveCameraSpeed * deltaTime;

		if (m_cameraForwardKeyPressed) {
			m_camera.position += m_camera.perspectiveDirection * cameraSpeed;
		}
		if (m_cameraBackwardKeyPressed) {
			m_camera.position -= m_camera.perspectiveDirection * cameraSpeed;
		}
		if (m_cameraLeftKeyPressed) {
			m_camera.position -= t * cameraSpeed;
		}
		if (m_cameraRightKeyPressed) {
			m_camera.position += t * cameraSpeed;
		}
		if (m_cameraUpKeyPressed) {
			m_camera.position.y += cameraSpeed;
		}
		if (m_cameraDownKeyPressed) {
			m_camera.position.y -= cameraSpeed;
		}

		m_camera.viewMatrix = nml::lookAtRH(m_camera.position, m_camera.position + m_camera.perspectiveDirection, m_camera.perspectiveUp);
		m_camera.projectionMatrix = perspectiveRHOpenGL(nml::toRad(45.0f), static_cast<float>(width()) / static_cast<float>(height()), m_globalInfo.editorParameters.renderer.cameraNearPlane, m_globalInfo.editorParameters.renderer.cameraFarPlane);
	}
	else {
		nml::vec3 t;
		if ((m_camera.orthographicDirection.y == 1.0f) || (m_camera.orthographicDirection.y == -1.0f)) {
			t = nml::normalize(nml::vec3(-m_camera.orthographicDirection.y, 0.0f, m_camera.orthographicDirection.x));
		}
		else {
			t = nml::normalize(nml::vec3(-m_camera.orthographicDirection.z, 0.0f, m_camera.orthographicDirection.x));
		}

		float horizontalSpeed = m_camera.orthographicHalfExtent * m_globalInfo.editorParameters.renderer.orthographicCameraSpeed * deltaTime * ((m_mouseCursorDifference.x == 0.0f) ? 1.0f : std::abs(m_mouseCursorDifference.x));
		float verticalSpeed = m_camera.orthographicHalfExtent * m_globalInfo.editorParameters.renderer.orthographicCameraSpeed * deltaTime * ((m_mouseCursorDifference.y == 0.0f) ? 1.0f : std::abs(m_mouseCursorDifference.y));
		float halfExtentSpeed = m_globalInfo.editorParameters.renderer.orthographicCameraSpeed * 5.0f * ((m_mouseScrollY == 0.0f) ? 1.0f : 2.0f) * deltaTime;

		if (m_cameraForwardKeyPressed || (m_mouseCursorDifference.y < 0.0f)) {
			m_camera.position += m_camera.orthographicUp * verticalSpeed;
		}
		if (m_cameraBackwardKeyPressed || (m_mouseCursorDifference.y > 0.0f)) {
			m_camera.position -= m_camera.orthographicUp * verticalSpeed;
		}
		if (m_cameraLeftKeyPressed || (m_mouseCursorDifference.x < 0.0f)) {
			m_camera.position -= t * horizontalSpeed;
		}
		if (m_cameraRightKeyPressed || (m_mouseCursorDifference.x > 0.0f)) {
			m_camera.position += t * horizontalSpeed;
		}
		if (m_cameraUpKeyPressed || (m_mouseScrollY < 0.0f)) {
			m_camera.orthographicHalfExtent += halfExtentSpeed;
		}
		if (m_cameraDownKeyPressed || (m_mouseScrollY > 0.0f)) {
			m_camera.orthographicHalfExtent -= halfExtentSpeed;
			m_camera.orthographicHalfExtent = std::max(m_camera.orthographicHalfExtent, 0.01f);
		}

		m_camera.viewMatrix = nml::lookAtRH(m_camera.position, m_camera.position + m_camera.orthographicDirection, m_camera.orthographicUp);
		float orthographicHalfExtentWidth = m_camera.orthographicHalfExtent * static_cast<float>(width()) / static_cast<float>(height());
		m_camera.projectionMatrix = orthographicRHOpenGL(-orthographicHalfExtentWidth, orthographicHalfExtentWidth, -m_camera.orthographicHalfExtent, m_camera.orthographicHalfExtent, -m_globalInfo.editorParameters.renderer.cameraFarPlane, m_globalInfo.editorParameters.renderer.cameraFarPlane);
	}

	m_camera.viewProjMatrix = m_camera.projectionMatrix * m_camera.viewMatrix;
	m_camera.invViewMatrix = nml::inverse(m_camera.viewMatrix);
	m_camera.invProjMatrix = nml::inverse(m_camera.projectionMatrix);

	m_mouseCursorDifference = nml::vec2(0.0f, 0.0f);

	m_mouseScrollY = 0.0f;
}

void Renderer::updateLights() {
	gl.glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_lightBuffer);
	glex.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, glex.glGetProgramResourceIndex(m_entityProgram, GL_SHADER_STORAGE_BLOCK, "LightBuffer"), m_lightBuffer);

	std::array<uint32_t, 4> lightsCount = { 0, 0, 0, 0 };
	std::vector<nml::vec4> directionalLightsInfos;
	std::vector<nml::vec4> pointLightsInfos;
	std::vector<nml::vec4> spotLightsInfos;
	std::vector<nml::vec4> ambientLightsInfos;

	for (const auto& entity : m_globalInfo.entities) {
		if (entity.second.light) {
			bool hasEntityMoveTransform = m_entityMoveTransforms.find(entity.second.entityID) != m_entityMoveTransforms.end();
			const Transform& transform = hasEntityMoveTransform ? m_entityMoveTransforms[entity.second.entityID] : entity.second.transform;
			const Light& light = entity.second.light.value();

			if (light.type == "Directional") {
				lightsCount[0]++;

				const nml::vec3 baseLightDirection = nml::normalize(light.direction);
				const float baseDirectionYaw = std::atan2(baseLightDirection.z, baseLightDirection.x);
				const float baseDirectionPitch = -std::asin(baseLightDirection.y);
				const nml::vec3 lightDirection = nml::normalize(nml::vec3(
					std::cos(baseDirectionPitch + nml::toRad(transform.rotation.x)) * std::cos(baseDirectionYaw + nml::toRad(transform.rotation.y)),
					-std::sin(baseDirectionPitch + nml::toRad(transform.rotation.x)),
					std::cos(baseDirectionPitch + nml::toRad(transform.rotation.x)) * std::sin(baseDirectionYaw + nml::toRad(transform.rotation.y))
				));

				directionalLightsInfos.push_back(nml::vec4());
				directionalLightsInfos.push_back(nml::vec4(lightDirection, 0.0f));
				directionalLightsInfos.push_back(nml::vec4(light.color, light.intensity));
				directionalLightsInfos.push_back(nml::vec4());
			}
			else if (light.type == "Point") {
				lightsCount[1]++;

				pointLightsInfos.push_back(nml::vec4(transform.position, 0.0f));
				pointLightsInfos.push_back(nml::vec4());
				pointLightsInfos.push_back(nml::vec4(light.color, light.intensity));
				pointLightsInfos.push_back(nml::vec4(0.0f, 0.0f, light.distance, 0.0f));
			}
			else if (light.type == "Spot") {
				lightsCount[2]++;

				const nml::vec3 baseLightDirection = nml::normalize(light.direction);
				const float baseDirectionYaw = std::atan2(baseLightDirection.z, baseLightDirection.x);
				const float baseDirectionPitch = -std::asin(baseLightDirection.y);
				const nml::vec3 lightDirection = nml::normalize(nml::vec3(
					std::cos(baseDirectionPitch + nml::toRad(transform.rotation.x)) * std::cos(baseDirectionYaw + nml::toRad(transform.rotation.y)),
					-std::sin(baseDirectionPitch + nml::toRad(transform.rotation.x)),
					std::cos(baseDirectionPitch + nml::toRad(transform.rotation.x)) * std::sin(baseDirectionYaw + nml::toRad(transform.rotation.y))
				));

				spotLightsInfos.push_back(nml::vec4(transform.position, 0.0f));
				spotLightsInfos.push_back(nml::vec4(lightDirection, 0.0f));
				spotLightsInfos.push_back(nml::vec4(light.color, light.intensity));
				spotLightsInfos.push_back(nml::vec4(nml::toRad(light.cutoff.x), nml::toRad(light.cutoff.y), light.distance, 0.0f));
			}
			else if (light.type == "Ambient") {
				lightsCount[3]++;

				ambientLightsInfos.push_back(nml::vec4());
				ambientLightsInfos.push_back(nml::vec4());
				ambientLightsInfos.push_back(nml::vec4(light.color, light.intensity));
				ambientLightsInfos.push_back(nml::vec4());
			}
		}
	}

	gl.glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 4 * sizeof(uint32_t), lightsCount.data());
	gl.glBufferSubData(GL_SHADER_STORAGE_BUFFER, 4 * sizeof(uint32_t), directionalLightsInfos.size() * sizeof(nml::vec4), directionalLightsInfos.data());
	gl.glBufferSubData(GL_SHADER_STORAGE_BUFFER, 4 * sizeof(uint32_t) + directionalLightsInfos.size() * sizeof(nml::vec4), pointLightsInfos.size() * sizeof(nml::vec4), pointLightsInfos.data());
	gl.glBufferSubData(GL_SHADER_STORAGE_BUFFER, 4 * sizeof(uint32_t) + directionalLightsInfos.size() * sizeof(nml::vec4) + pointLightsInfos.size() * sizeof(nml::vec4), spotLightsInfos.size() * sizeof(nml::vec4), spotLightsInfos.data());
	gl.glBufferSubData(GL_SHADER_STORAGE_BUFFER, 4 * sizeof(uint32_t) + directionalLightsInfos.size() * sizeof(nml::vec4) + pointLightsInfos.size() * sizeof(nml::vec4) + spotLightsInfos.size() * sizeof(nml::vec4), ambientLightsInfos.size() * sizeof(nml::vec4), ambientLightsInfos.data());
}

void Renderer::loadResourcesToGPU() {
	for (const std::string& modelToLoad : m_globalInfo.rendererResourceManager.modelsToLoad) {
		if (m_globalInfo.rendererResourceManager.rendererModels.find(modelToLoad) != m_globalInfo.rendererResourceManager.rendererModels.end()) {
			// Free resources for same model
			for (auto& primitive : m_globalInfo.rendererResourceManager.rendererModels[modelToLoad].primitives) {
				gl.glDeleteBuffers(1, &primitive.mesh.vertexBuffer);
				gl.glDeleteBuffers(1, &primitive.mesh.indexBuffer);
			}
		}

		const RendererResourceManager::Model& model = m_globalInfo.rendererResourceManager.models[modelToLoad];
		RendererModel newRendererModel;
		for (const auto& modelPrimitive : model.primitives) {
			const RendererResourceManager::Mesh& mesh = modelPrimitive.mesh;
			const RendererMaterial& material = modelPrimitive.material;

			RendererPrimitive newRendererPrimitive;
			gl.glGenBuffers(1, &newRendererPrimitive.mesh.vertexBuffer);
			gl.glBindBuffer(GL_ARRAY_BUFFER, newRendererPrimitive.mesh.vertexBuffer);
			gl.glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(RendererResourceManager::Mesh::Vertex), mesh.vertices.data(), GL_STATIC_DRAW);
			
			gl.glGenBuffers(1, &newRendererPrimitive.mesh.indexBuffer);
			gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, newRendererPrimitive.mesh.indexBuffer);
			gl.glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(uint32_t), mesh.indices.data(), GL_STATIC_DRAW);

			newRendererPrimitive.mesh.indexCount = static_cast<uint32_t>(mesh.indices.size());

			newRendererPrimitive.material = material;

			newRendererModel.primitives.push_back(newRendererPrimitive);
		}

		m_globalInfo.rendererResourceManager.rendererModels[modelToLoad] = newRendererModel;
	}
	m_globalInfo.rendererResourceManager.modelsToLoad.clear();

	for (const auto& imageToGPU : m_globalInfo.rendererResourceManager.imagesToGPU) {
		if (m_globalInfo.rendererResourceManager.textures.find(imageToGPU.first) != m_globalInfo.rendererResourceManager.textures.end()) {
			// Free resources for same image
			gl.glDeleteTextures(1, &m_globalInfo.rendererResourceManager.textures[imageToGPU.first]);
		}

		m_globalInfo.rendererResourceManager.textures[imageToGPU.first] = 0;
		uint32_t* texture = &m_globalInfo.rendererResourceManager.textures[imageToGPU.first];
		gl.glGenTextures(1, texture);
		gl.glBindTexture(GL_TEXTURE_2D, *texture);
		gl.glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, imageToGPU.second.width, imageToGPU.second.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageToGPU.second.data.data());
		gl.glGenerateMipmap(GL_TEXTURE_2D);
	}
	m_globalInfo.rendererResourceManager.imagesToGPU.clear();

	for (const auto& samplerToGPU : m_globalInfo.rendererResourceManager.samplersToGPU) {
		RendererSampler sampler;
		if (samplerToGPU.second.magFilter == RendererResourceManager::SamplerToGPU::Filter::Nearest) {
			sampler.magFilter = GL_NEAREST;
		}
		else {
			sampler.magFilter = GL_LINEAR;
		}

		if (samplerToGPU.second.minFilter == RendererResourceManager::SamplerToGPU::Filter::Nearest) {
			sampler.minFilter = GL_NEAREST;
		}
		else {
			sampler.minFilter = GL_LINEAR;
		}

		if (samplerToGPU.second.mipmapFilter == RendererResourceManager::SamplerToGPU::Filter::Nearest) {
			sampler.mipmapFilter = GL_NEAREST;
		}
		else {
			sampler.mipmapFilter = GL_LINEAR;
		}

		if (samplerToGPU.second.wrapS == RendererResourceManager::SamplerToGPU::Wrap::ClampToEdge) {
			sampler.wrapS = GL_CLAMP_TO_EDGE;
		}
		else if (samplerToGPU.second.wrapS == RendererResourceManager::SamplerToGPU::Wrap::Repeat) {
			sampler.wrapS = GL_REPEAT;
		}
		else {
			sampler.wrapS = GL_MIRRORED_REPEAT;
		}

		if (samplerToGPU.second.wrapT == RendererResourceManager::SamplerToGPU::Wrap::ClampToEdge) {
			sampler.wrapT = GL_CLAMP_TO_EDGE;
		}
		else if (samplerToGPU.second.wrapS == RendererResourceManager::SamplerToGPU::Wrap::Repeat) {
			sampler.wrapT = GL_REPEAT;
		}
		else {
			sampler.wrapT = GL_MIRRORED_REPEAT;
		}

		m_globalInfo.rendererResourceManager.samplers[samplerToGPU.first] = sampler;
	}
	m_globalInfo.rendererResourceManager.samplersToGPU.clear();
}

void Renderer::calculateTranslation(const std::set<EntityID> entityIDs, const nml::vec2& mouseCursorCurrentPosition) {
	nml::vec3 translationAxis = nml::vec3(0.0f, 0.0f, 0.0f);
	uint8_t gizmoAxisIndex = 255;
	if (m_gizmoAxis == GizmoAxis::X) {
		translationAxis.x = 1.0f;
		gizmoAxisIndex = 0;
	}
	else if (m_gizmoAxis == GizmoAxis::Y) {
		translationAxis.y = 1.0f;
		gizmoAxisIndex = 1;
	}
	else if (m_gizmoAxis == GizmoAxis::Z) {
		translationAxis.z = 1.0f;
		gizmoAxisIndex = 2;
	}

	nml::vec3 cameraEntityDifference = m_selectionMeanPosition - m_camera.position;
	float cameraEntityDifferenceLength = (nml::dot(cameraEntityDifference, cameraEntityDifference) != 0.0f) ? cameraEntityDifference.length() : 0.0f;
	nml::vec3 worldSpaceCursorCurrentPosition = unproject(mouseCursorCurrentPosition, static_cast<float>(width()), static_cast<float>(height()), m_camera.invViewMatrix, m_camera.invProjMatrix);
	nml::vec3 worldSpaceCursorPreviousPosition = unproject(m_mouseCursorPreviousPosition, static_cast<float>(width()), static_cast<float>(height()), m_camera.invViewMatrix, m_camera.invProjMatrix);
	nml::vec3 worldSpaceCursorDifference = worldSpaceCursorCurrentPosition - worldSpaceCursorPreviousPosition;
	if ((m_gizmoMode == GizmoMode::Translate) && !m_translateEntityMode) {
		worldSpaceCursorDifference = nml::vec3(worldSpaceCursorDifference.x * translationAxis.x, worldSpaceCursorDifference.y * translationAxis.y, worldSpaceCursorDifference.z * translationAxis.z);
	}
	float worldSpaceCursorDifferenceLength = (nml::dot(worldSpaceCursorDifference, worldSpaceCursorDifference) != 0.0f) ? worldSpaceCursorDifference.length() : 0.0f;
	float nearPlane = (m_globalInfo.editorParameters.renderer.cameraNearPlane != 0.0f) ? m_globalInfo.editorParameters.renderer.cameraNearPlane : 0.1f;
	float coefficient = (cameraEntityDifferenceLength * worldSpaceCursorDifferenceLength) / nearPlane;
	if ((m_gizmoMode == GizmoMode::Translate) && !m_translateEntityMode && (QGuiApplication::keyboardModifiers() == Qt::ShiftModifier) && (m_globalInfo.editorParameters.renderer.gizmoTranslationStep[gizmoAxisIndex] > 0.0f)) {
		if (!m_camera.useOrthographicProjection) {
			m_gizmoTranslationStepAccumulation += coefficient;
		}
		else {
			m_gizmoTranslationStepAccumulation += worldSpaceCursorDifferenceLength;
		}
		float positionStepDifference = 0.0f;
		float positionSign = (worldSpaceCursorDifference[gizmoAxisIndex] > 0.0f) ? 1.0f : -1.0f;
		while (m_gizmoTranslationStepAccumulation >= m_globalInfo.editorParameters.renderer.gizmoTranslationStep[gizmoAxisIndex]) {
			positionStepDifference += positionSign * m_globalInfo.editorParameters.renderer.gizmoTranslationStep[gizmoAxisIndex];
			m_gizmoTranslationStepAccumulation -= m_globalInfo.editorParameters.renderer.gizmoTranslationStep[gizmoAxisIndex];
		}
		for (EntityID entityID : entityIDs) {
			m_entityMoveTransforms[entityID].position[gizmoAxisIndex] += positionStepDifference;
		}
	}
	else {
		nml::vec3 worldSpaceCursorDifferenceNormalized = (worldSpaceCursorDifferenceLength != 0.0f) ? nml::normalize(worldSpaceCursorDifference) : nml::vec3(0.0f, 0.0f, 0.0f);
		for (EntityID entityID : entityIDs) {
			if (!m_camera.useOrthographicProjection) {
				m_entityMoveTransforms[entityID].position += worldSpaceCursorDifferenceNormalized * coefficient;
			}
			else {
				m_entityMoveTransforms[entityID].position += worldSpaceCursorDifference;
			}
		}
	}
}

void Renderer::calculateRotation(const std::set<EntityID> entityIDs, const nml::vec2& mouseCursorCurrentPosition) {
	nml::vec3 rotationAxis = nml::vec3(0.0f, 0.0f, 0.0f);
	uint8_t gizmoAxisIndex = 255;
	if ((m_gizmoMode == GizmoMode::Rotate) && !m_rotateEntityMode) {
		if (m_gizmoAxis == GizmoAxis::X) {
			rotationAxis.x = 1.0f;
			gizmoAxisIndex = 0;
		}
		else if (m_gizmoAxis == GizmoAxis::Y) {
			rotationAxis.y = 1.0f;
			gizmoAxisIndex = 1;
		}
		else if (m_gizmoAxis == GizmoAxis::Z) {
			rotationAxis.z = 1.0f;
			gizmoAxisIndex = 2;
		}
	}
	else {
		rotationAxis = (!m_camera.useOrthographicProjection) ? m_camera.perspectiveDirection : m_camera.orthographicDirection;
	}
	float mouseCursorDifferenceX = mouseCursorCurrentPosition.x - m_mouseCursorPreviousPosition.x;
	float mouseCursorDifferenceY = mouseCursorCurrentPosition.y - m_mouseCursorPreviousPosition.y;
	float mouseCursorDifference = 0.0f;
	float size = 1.0f;
	if (std::abs(mouseCursorDifferenceX) > std::abs(mouseCursorDifferenceY)) {
		mouseCursorDifference = mouseCursorDifferenceX;
		size = width();
	}
	else {
		mouseCursorDifference = mouseCursorDifferenceY;
		size = height();
	}
	nml::mat4 rotationMatrix = nml::rotate(mouseCursorDifference / size, rotationAxis);
	nml::vec3 rotationAngles = nml::rotationMatrixToEulerAngles(rotationMatrix);
	rotationAngles.x = nml::toDeg(rotationAngles.x);
	rotationAngles.y = nml::toDeg(rotationAngles.y);
	rotationAngles.z = nml::toDeg(rotationAngles.z);
	if ((m_gizmoMode == GizmoMode::Rotate) && !m_rotateEntityMode && (QGuiApplication::keyboardModifiers() == Qt::ShiftModifier) && (m_globalInfo.editorParameters.renderer.gizmoRotationStep[gizmoAxisIndex] > 0.0f)) {
		m_gizmoRotationStepAccumulation += std::abs(rotationAngles[gizmoAxisIndex]);
		float rotationStepDifference = 0.0f;
		float mouseCursorDifferenceSign = (mouseCursorDifference > 0.0f) ? 1.0f : -1.0f;
		while (m_gizmoRotationStepAccumulation >= m_globalInfo.editorParameters.renderer.gizmoRotationStep[gizmoAxisIndex]) {
			rotationStepDifference += m_globalInfo.editorParameters.renderer.gizmoRotationStep[gizmoAxisIndex];
			m_gizmoRotationStepAccumulation -= m_globalInfo.editorParameters.renderer.gizmoRotationStep[gizmoAxisIndex];
		}
		nml::mat4 stepRotationMatrix = nml::rotate(mouseCursorDifferenceSign * nml::toRad(rotationStepDifference), rotationAxis);
		for (EntityID entityID : entityIDs) {
			if (entityIDs.size() != 1) {
				m_entityMoveTransforms[entityID].position = nml::vec3(nml::translate(m_selectionMeanPosition) * stepRotationMatrix * nml::translate(-m_selectionMeanPosition) * nml::vec4(m_entityMoveTransforms[entityID].position, 1.0f));
			}
			m_entityMoveTransforms[entityID].rotation[gizmoAxisIndex] += mouseCursorDifferenceSign * rotationStepDifference;
			m_entityMoveTransforms[entityID].rotation[gizmoAxisIndex] = std::fmod(m_entityMoveTransforms[entityID].rotation[gizmoAxisIndex], 360.0f);
		}
	}
	else {
		for (EntityID entityID : entityIDs) {
			if (entityIDs.size() != 1) {
				m_entityMoveTransforms[entityID].position = nml::vec3(nml::translate(m_selectionMeanPosition) * rotationMatrix * nml::translate(-m_selectionMeanPosition) * nml::vec4(m_entityMoveTransforms[entityID].position, 1.0f));
			}
			m_entityMoveTransforms[entityID].rotation += rotationAngles;
			m_entityMoveTransforms[entityID].rotation = nml::vec3(std::fmod(m_entityMoveTransforms[entityID].rotation.x, 360.0f), std::fmod(m_entityMoveTransforms[entityID].rotation.y, 360.0f), std::fmod(m_entityMoveTransforms[entityID].rotation.z, 360.0f));
		}
	}
}

void Renderer::calculateScale(const std::set<EntityID> entityIDs, const nml::vec2& mouseCursorCurrentPosition) {
	size_t gizmoAxisIndex = 0;
	if ((m_gizmoMode == GizmoMode::Scale) && !m_scaleEntityMode) {
		if (m_gizmoAxis == GizmoAxis::X) {
			gizmoAxisIndex = 0;
		}
		else if (m_gizmoAxis == GizmoAxis::Y) {
			gizmoAxisIndex = 1;
		}
		else if (m_gizmoAxis == GizmoAxis::Z) {
			gizmoAxisIndex = 2;
		}
	}
	nml::vec2 previousToCurrentMousePosition = mouseCursorCurrentPosition - m_mouseCursorPreviousPosition;
	nml::vec3 worldSpacePreviousMouse = unproject(m_mouseCursorPreviousPosition, static_cast<float>(width()), static_cast<float>(height()), m_camera.invViewMatrix, m_camera.invProjMatrix);
	nml::vec3 worldSpaceCurrentMouse = unproject(mouseCursorCurrentPosition, static_cast<float>(width()), static_cast<float>(height()), m_camera.invViewMatrix, m_camera.invProjMatrix);
	nml::vec2 objectPositionProjected = project(m_selectionMeanPosition, static_cast<float>(width()), static_cast<float>(height()), m_camera.viewProjMatrix);
	nml::vec2 objectToCurrentMousePosition = mouseCursorCurrentPosition - objectPositionProjected;
	nml::vec3 previousToCurrentPosition3D = worldSpaceCurrentMouse - worldSpacePreviousMouse;
	float previousToCurrentPosition3DLength = (nml::dot(previousToCurrentPosition3D, previousToCurrentPosition3D) != 0.0f) ? previousToCurrentPosition3D.length() : 0.0f;
	nml::vec3 objectToCurrentMousePosition3D = worldSpaceCurrentMouse - m_selectionMeanPosition;
	float objectToCurrentMousePosition3DLength = (nml::dot(objectToCurrentMousePosition3D, objectToCurrentMousePosition3D) != 0.0f) ? objectToCurrentMousePosition3D.length() : 0.01f;
	float scaleFactor = 1.0f;
	if (!m_camera.useOrthographicProjection) {
		scaleFactor = 1000.0f;
	}
	float scaleSign = ((nml::dot(previousToCurrentMousePosition, objectToCurrentMousePosition) > 0.0) ? 1.0f : -1.0f);
	if ((m_gizmoMode == GizmoMode::Scale)) {
		if (m_gizmoAxis == GizmoAxis::X) {
			scaleSign = (nml::dot(previousToCurrentPosition3D, nml::vec3(1.0f, 0.0f, 0.0f)) > 0.0) ? 1.0f : -1.0f;
		}
		else if (m_gizmoAxis == GizmoAxis::Y) {
			scaleSign = (nml::dot(previousToCurrentPosition3D, nml::vec3(0.0f, 1.0f, 0.0f)) > 0.0) ? 1.0f : -1.0f;
		}
		else if (m_gizmoAxis == GizmoAxis::Z) {
			scaleSign = (nml::dot(previousToCurrentPosition3D, nml::vec3(0.0f, 0.0f, 1.0f)) > 0.0) ? 1.0f : -1.0f;
		}
	}
	float scaleDifference = ((previousToCurrentPosition3DLength * scaleFactor) / objectToCurrentMousePosition3DLength) * scaleSign;
	if ((m_gizmoMode == GizmoMode::Scale) && !m_rotateEntityMode && (QGuiApplication::keyboardModifiers() == Qt::ShiftModifier) && (m_globalInfo.editorParameters.renderer.gizmoRotationStep[gizmoAxisIndex] > 0.0f)) {
		m_gizmoScaleStepAccumulation += std::abs(scaleDifference);
		float scaleStepDifference = 0.0f;
		while (m_gizmoScaleStepAccumulation >= m_globalInfo.editorParameters.renderer.gizmoScaleStep[gizmoAxisIndex]) {
			scaleStepDifference += m_globalInfo.editorParameters.renderer.gizmoScaleStep[gizmoAxisIndex];
			m_gizmoScaleStepAccumulation -= m_globalInfo.editorParameters.renderer.gizmoScaleStep[gizmoAxisIndex];
		}
		for (EntityID entityID : entityIDs) {
			m_entityMoveTransforms[entityID].position[gizmoAxisIndex] = (1.0f + (scaleSign * scaleStepDifference)) * (m_entityMoveTransforms[entityID].position[gizmoAxisIndex] - m_selectionMeanPosition[gizmoAxisIndex]) + m_selectionMeanPosition[gizmoAxisIndex];
			m_entityMoveTransforms[entityID].scale[gizmoAxisIndex] += scaleSign * scaleStepDifference;
		}
	}
	else {
		for (EntityID entityID : entityIDs) {
			if ((m_gizmoMode == GizmoMode::Scale) && !m_scaleEntityMode) {
				m_entityMoveTransforms[entityID].position[gizmoAxisIndex] = (1.0f + scaleDifference) * (m_entityMoveTransforms[entityID].position[gizmoAxisIndex] - m_selectionMeanPosition[gizmoAxisIndex]) + m_selectionMeanPosition[gizmoAxisIndex];
				m_entityMoveTransforms[entityID].scale[gizmoAxisIndex] *= 1.0f + scaleDifference;
			}
			else {
				m_entityMoveTransforms[entityID].position = (1.0f + scaleDifference) * (m_entityMoveTransforms[entityID].position - m_selectionMeanPosition) + m_selectionMeanPosition;
				m_entityMoveTransforms[entityID].scale *= 1.0f + scaleDifference;
			}
		}
	}
}

void Renderer::startTransform() {
	m_entityMoveTransforms[m_globalInfo.currentEntityID] = m_globalInfo.entities[m_globalInfo.currentEntityID].transform;
	for (EntityID otherSelectedEntityIDs : m_globalInfo.otherSelectedEntityIDs) {
		m_entityMoveTransforms[otherSelectedEntityIDs] = m_globalInfo.entities[otherSelectedEntityIDs].transform;
	}
	QPoint cursorPos = mapFromGlobal(QCursor::pos());
	m_mouseCursorPreviousPosition = nml::vec2(static_cast<float>(cursorPos.x()), static_cast<float>(height() - cursorPos.y()));
	m_selectionMeanPosition = m_entityMoveTransforms[m_globalInfo.currentEntityID].position;
	for (EntityID otherSelectedEntityID : m_globalInfo.otherSelectedEntityIDs) {
		m_selectionMeanPosition += m_entityMoveTransforms[otherSelectedEntityID].position;
	}
	m_selectionMeanPosition /= static_cast<float>(m_globalInfo.otherSelectedEntityIDs.size() + 1);
}

void Renderer::cancelTransform() {
	if (anyEntityTransformMode()) {
		m_translateEntityMode = false;
		m_rotateEntityMode = false;
		m_scaleEntityMode = false;
		m_selectionMeanPosition = nml::vec3(0.0f, 0.0f, 0.0f);
		m_entityMoveTransforms.clear();
	}
}

nml::vec2 Renderer::project(const nml::vec3& p, float width, float height, const nml::mat4& viewProjMatrix) {
	nml::vec4 clipSpace = viewProjMatrix * nml::vec4(p, 1.0f);
	clipSpace /= clipSpace.w;
	nml::vec2 screenSpace = (clipSpace + nml::vec2(1.0f, 1.0f)) / 2.0f;

	return nml::vec2(screenSpace.x * width, screenSpace.y * height);
}

nml::vec3 Renderer::unproject(const nml::vec2& p, float width, float height, const nml::mat4& invViewMatrix, const nml::mat4& invProjMatrix) {
	nml::vec2 screenSpace = nml::vec2(p.x / width, p.y / height);
	nml::vec2 clipSpace = (screenSpace * 2.0f) - 1.0f;
	nml::vec4 viewSpace = invProjMatrix * nml::vec4(clipSpace, 0.0f, 1.0f);
	nml::vec4 worldSpace = invViewMatrix * viewSpace;

	return nml::vec3(worldSpace) / worldSpace.w;
}

nml::mat4 Renderer::perspectiveRHOpenGL(float fovY, float aspectRatio, float near, float far) {
	const float tanHalfFovY = std::tan(fovY / 2.0f);
	const float farMinusNear = far - near;
		
	return nml::mat4(1.0f / (aspectRatio * tanHalfFovY), 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f / tanHalfFovY, 0.0f, 0.0f,
		0.0f, 0.0f, -(far + near) / (farMinusNear), -1.0f,
		0.0f, 0.0f, -(2.0f * far * near) / (farMinusNear), 0.0f);
}

nml::mat4 Renderer::orthographicRHOpenGL(float left, float right, float bottom, float top, float near, float far) {
	const float rightPlusLeft = right + left;
	const float rightMinusLeft = right - left;
	const float topPlusBottom = top + bottom;
	const float topMinusBottom = top - bottom;
	const float farMinusNear = far - near;

	return nml::mat4(2.0f / rightMinusLeft, 0.0f, 0.0f, 0.0f,
		0.0f, 2.0f / topMinusBottom, 0.0f, 0.0f,
		0.0f, 0.0f, -1.0f / farMinusNear, 0.0f,
		-(rightPlusLeft / rightMinusLeft), -(topPlusBottom / topMinusBottom), -(far + near) / farMinusNear, 1.0f);
}

void Renderer::onEntityDestroyed(EntityID entityID) {
	(void)entityID;
	cancelTransform();
}

void Renderer::onEntitySelected() {
	cancelTransform();
}

void Renderer::onCameraProjectionSwitched(bool cameraProjectionOrthographic) {
	cancelTransform();

	m_camera.useOrthographicProjection = cameraProjectionOrthographic;
}

void Renderer::onCameraReset() {
	cancelTransform();

	if (!m_camera.useOrthographicProjection) {
		m_camera.position = m_camera.basePerspectivePosition;
		m_camera.perspectiveDirection = m_camera.basePerspectiveDirection;
		m_camera.perspectiveYaw = nml::toDeg(std::atan2(m_camera.perspectiveDirection[2], m_camera.perspectiveDirection[0]));
		m_camera.perspectivePitch = nml::toDeg(-std::asin(m_camera.perspectiveDirection[1]));
	}
	else {
		m_camera.position = m_camera.baseOrthographicPosition;
		m_camera.orthographicDirection = m_camera.baseOrthographicDirection;
		m_camera.orthographicUp = m_camera.baseOrthographicUp;
		m_camera.orthographicHalfExtent = m_camera.baseOrthographicHalfExtent;
	}
}

void Renderer::onOrthographicCameraToAxisChanged(const nml::vec3& axis) {
	cancelTransform();

	m_camera.useOrthographicProjection = true;
	m_camera.orthographicDirection = axis;
	if ((axis.y == -1.0f) || (axis.y == 1.0f)) {
		m_camera.orthographicUp = nml::vec3(0.0f, 0.0f, -1.0f);
	}
	else {
		m_camera.orthographicUp = nml::vec3(0.0f, 1.0f, 0.0f);
	}
}

void Renderer::onCameraGoToEntity(EntityID entityID) {
	if (m_camera.useOrthographicProjection) {
		m_camera.position = m_globalInfo.entities[entityID].transform.position;
	}
	else {
		m_camera.position = m_globalInfo.entities[entityID].transform.position + nml::vec3(0.0f, 3.0f, -3.0f);
		m_camera.perspectiveDirection = nml::normalize(nml::vec3(0.0f, -1.0f, 1.0f));
		m_camera.perspectiveYaw = nml::toDeg(std::atan2(m_camera.perspectiveDirection[2], m_camera.perspectiveDirection[0]));
		m_camera.perspectivePitch = nml::toDeg(-std::asin(m_camera.perspectiveDirection[1]));
	}
}

void Renderer::keyPressEvent(QKeyEvent* event) {
	if (event->isAutoRepeat()) {
		event->accept();
		return;
	}

	if (event->key() == m_globalInfo.editorParameters.renderer.cameraForwardKey) {
		m_cameraForwardKeyPressed = true;
	}
	else if (event->key() == m_globalInfo.editorParameters.renderer.cameraBackwardKey) {
		m_cameraBackwardKeyPressed = true;
	}
	else if (event->key() == m_globalInfo.editorParameters.renderer.cameraLeftKey) {
		m_cameraLeftKeyPressed = true;
	}
	else if (event->key() == m_globalInfo.editorParameters.renderer.cameraRightKey) {
		m_cameraRightKeyPressed = true;
	}
	else if (event->key() == m_globalInfo.editorParameters.renderer.cameraUpKey) {
		m_cameraUpKeyPressed = true;
	}
	else if (event->key() == m_globalInfo.editorParameters.renderer.cameraDownKey) {
		m_cameraDownKeyPressed = true;
	}
	else if (event->key() == m_globalInfo.editorParameters.renderer.translateEntityKey) {
		if (QGuiApplication::keyboardModifiers() == Qt::ControlModifier) {
			if ((m_globalInfo.currentEntityID != NO_ENTITY) && !m_moveCameraButtonPressed && !anyEntityTransformMode()) {
				m_translateEntityMode = true;
				startTransform();
			}
		}
		else {
			if (m_gizmoAxis == GizmoAxis::None) {
				if (m_gizmoMode != GizmoMode::Translate) {
						m_gizmoMode = GizmoMode::Translate;
				}
				else {
					m_gizmoMode = GizmoMode::None;
				}
			}
		}
	}
	else if (event->key() == m_globalInfo.editorParameters.renderer.rotateEntityKey) {
		if (QGuiApplication::keyboardModifiers() == Qt::ControlModifier) {
			if ((m_globalInfo.currentEntityID != NO_ENTITY) && !m_moveCameraButtonPressed && !anyEntityTransformMode()) {
				m_rotateEntityMode = true;
				startTransform();
			}
		}
		else {
			if (m_gizmoAxis == GizmoAxis::None) {
				if (m_gizmoMode != GizmoMode::Rotate) {
					m_gizmoMode = GizmoMode::Rotate;
				}
				else {
					m_gizmoMode = GizmoMode::None;
				}
			}
		}
	}
	else if (event->key() == m_globalInfo.editorParameters.renderer.scaleEntityKey) {
		if (QGuiApplication::keyboardModifiers() == Qt::ControlModifier) {
			if ((m_globalInfo.currentEntityID != NO_ENTITY) && !m_moveCameraButtonPressed && !anyEntityTransformMode()) {
				m_scaleEntityMode = true;
				startTransform();
			}
		}
		else {
			if (m_gizmoAxis == GizmoAxis::None) {
				if (m_gizmoMode != GizmoMode::Scale) {
					m_gizmoMode = GizmoMode::Scale;
				}
				else {
					m_gizmoMode = GizmoMode::None;
				}
			}
		}
	}
	else if (event->key() == Qt::Key_Shift) {
		m_multiSelectionKeyPressed = true;
	}
	else if (event->key() == Qt::Key_Delete) {
		if (m_globalInfo.currentEntityID != NO_ENTITY) {
			std::vector<EntityID> entitiesToDestroy = { m_globalInfo.currentEntityID };
			std::copy(m_globalInfo.otherSelectedEntityIDs.begin(), m_globalInfo.otherSelectedEntityIDs.end(), std::back_inserter(entitiesToDestroy));
			m_globalInfo.actionUndoStack->push(new DestroyEntitiesCommand(m_globalInfo, entitiesToDestroy));
		}
	}
	else if (event->key() == Qt::Key_Control) {
		cancelTransform();
	}
	event->accept();
}

void Renderer::keyReleaseEvent(QKeyEvent* event) {
	if (event->isAutoRepeat()) {
		event->accept();
		return;
	}

	if (event->key() == m_globalInfo.editorParameters.renderer.cameraForwardKey) {
		m_cameraForwardKeyPressed = false;
	}
	else if (event->key() == m_globalInfo.editorParameters.renderer.cameraBackwardKey) {
		m_cameraBackwardKeyPressed = false;
	}
	else if (event->key() == m_globalInfo.editorParameters.renderer.cameraLeftKey) {
		m_cameraLeftKeyPressed = false;
	}
	else if (event->key() == m_globalInfo.editorParameters.renderer.cameraRightKey) {
		m_cameraRightKeyPressed = false;
	}
	else if (event->key() == m_globalInfo.editorParameters.renderer.cameraUpKey) {
		m_cameraUpKeyPressed = false;
	}
	else if (event->key() == m_globalInfo.editorParameters.renderer.cameraDownKey) {
		m_cameraDownKeyPressed = false;
	}
	else if (event->key() == Qt::Key_Shift) {
		m_multiSelectionKeyPressed = false;
	}
	event->accept();
}

void Renderer::mousePressEvent(QMouseEvent* event) {
	if (!anyEntityTransformMode()) {
		if (event->button() == Qt::LeftButton) {
			if (!m_moveCameraButtonPressed) {
				m_doPicking = true;
			}
		}
		else if (event->button() == Qt::RightButton) {
			if (m_gizmoAxis == GizmoAxis::None) {
				m_moveCameraButtonPressed = true;
				m_savedMousePosition = nml::vec2(static_cast<float>(QCursor::pos().x()), static_cast<float>(QCursor::pos().y()));
				m_mouseCursorPreviousPosition = m_savedMousePosition;
				setCursor(Qt::CursorShape::BlankCursor);
			}
		}
	}
	else {
		if (event->button() == Qt::LeftButton) {
			m_translateEntityMode = false;
			m_rotateEntityMode = false;
			m_scaleEntityMode = false;
			m_mouseCursorDifference = nml::vec2(0.0f, 0.0f);
			m_selectionMeanPosition = nml::vec3(0.0f, 0.0f, 0.0f);
			if (m_globalInfo.currentEntityID != NO_ENTITY) {
				std::vector<EntityID> changedEntityIDs = { m_globalInfo.currentEntityID };
				std::vector<Component*> changedEntityTransforms = { &m_entityMoveTransforms[m_globalInfo.currentEntityID] };
				for (EntityID otherSelectedEntityID : m_globalInfo.otherSelectedEntityIDs) {
					changedEntityIDs.push_back(otherSelectedEntityID);
					changedEntityTransforms.push_back(&m_entityMoveTransforms[otherSelectedEntityID]);
				}
				m_globalInfo.actionUndoStack->push(new ChangeEntitiesComponentCommand(m_globalInfo, changedEntityIDs, "Transform", changedEntityTransforms));
				m_entityMoveTransforms.clear();
			}
		}
		else if (event->button() == Qt::RightButton) {
			cancelTransform();
		}
	}
	event->accept();
}

void Renderer::mouseReleaseEvent(QMouseEvent* event) {
	if (!anyEntityTransformMode()) {
		if (event->button() == Qt::RightButton) {
			if (m_moveCameraButtonPressed) {
				m_moveCameraButtonPressed = false;
				setCursor(Qt::CursorShape::ArrowCursor);
				QCursor::setPos(static_cast<int>(m_savedMousePosition.x), static_cast<int>(m_savedMousePosition.y));
				m_mouseCursorPreviousPosition = nml::vec2(0.0f, 0.0f);
				m_mouseCursorDifference = nml::vec2(0.0f, 0.0f);
			}
		}
	}
	if (event->button() == Qt::LeftButton) {
		if (m_gizmoAxis != GizmoAxis::None) {
			m_gizmoAxis = GizmoAxis::None;
			m_gizmoTranslationStepAccumulation = 0.0f;
			m_gizmoRotationStepAccumulation = 0.0f;
			m_gizmoScaleStepAccumulation = 0.0f;

			if (m_globalInfo.currentEntityID != NO_ENTITY) {
				std::vector<EntityID> changedEntityIDs = { m_globalInfo.currentEntityID };
				std::vector<Component*> changedEntityTransforms = { &m_entityMoveTransforms[m_globalInfo.currentEntityID] };
				for (EntityID otherSelectedEntityID : m_globalInfo.otherSelectedEntityIDs) {
					changedEntityIDs.push_back(otherSelectedEntityID);
					changedEntityTransforms.push_back(&m_entityMoveTransforms[otherSelectedEntityID]);
				}
				m_globalInfo.actionUndoStack->push(new ChangeEntitiesComponentCommand(m_globalInfo, changedEntityIDs, "Transform", changedEntityTransforms));
				m_entityMoveTransforms.clear();
			}
		}
	}
	event->accept();
}

void Renderer::mouseMoveEvent(QMouseEvent* event) {
	if (!anyEntityTransformMode()) {
		if (event->buttons() & Qt::LeftButton) {
			if (!m_moveCameraButtonPressed) {
				if (m_gizmoAxis != GizmoAxis::None) {
					nml::vec2 mouseCursorCurrentPosition = nml::vec2(static_cast<float>(event->pos().x()), static_cast<float>(height()) - static_cast<float>(event->pos().y()));
					std::set<EntityID> selectedEntityIDs = m_globalInfo.otherSelectedEntityIDs;
					selectedEntityIDs.insert(m_globalInfo.currentEntityID);

					if (m_gizmoMode == GizmoMode::Translate) {
						calculateTranslation(selectedEntityIDs, mouseCursorCurrentPosition);
					}
					else if (m_gizmoMode == GizmoMode::Rotate) {
						calculateRotation(selectedEntityIDs, mouseCursorCurrentPosition);
					}
					else if (m_gizmoMode == GizmoMode::Scale) {
						calculateScale(selectedEntityIDs, mouseCursorCurrentPosition);
					}
					m_mouseCursorPreviousPosition = mouseCursorCurrentPosition;

					// Update Transform Widget
					MainWindow* mainWindow = m_globalInfo.mainWindow;
					mainWindow->infoPanel->entityInfoPanel->componentScrollArea->componentList->transformWidget->updateWidgets(m_entityMoveTransforms[m_globalInfo.currentEntityID]);
				}
			}
		}
		if (event->buttons() & Qt::RightButton) {
			if (m_moveCameraButtonPressed) {
				if (!m_mouseMoveFlag) {
					nml::vec2 mouseCursorCurrentPosition = nml::vec2(static_cast<float>(QCursor::pos().x()), static_cast<float>(QCursor::pos().y()));
					m_mouseCursorDifference = mouseCursorCurrentPosition - m_mouseCursorPreviousPosition;
					m_mouseMoveFlag = true;
					QCursor::setPos(static_cast<int>(m_savedMousePosition.x), static_cast<int>(m_savedMousePosition.y));
				}
				else {
					m_mouseMoveFlag = false;
				}
			}
		}
	}
	else {
		if (m_globalInfo.currentEntityID != NO_ENTITY) {
			std::set<EntityID> selectedEntityIDs = m_globalInfo.otherSelectedEntityIDs;
			selectedEntityIDs.insert(m_globalInfo.currentEntityID);
			nml::vec2 mouseCursorCurrentPosition = nml::vec2(static_cast<float>(event->pos().x()), static_cast<float>(height()) - static_cast<float>(event->pos().y()));
			if (m_translateEntityMode) {
				calculateTranslation(selectedEntityIDs, mouseCursorCurrentPosition);
			}
			else if (m_rotateEntityMode) {
				calculateRotation(selectedEntityIDs, mouseCursorCurrentPosition);
			}
			else if (m_scaleEntityMode) {
				calculateScale(selectedEntityIDs, mouseCursorCurrentPosition);
			}
			m_mouseCursorPreviousPosition = mouseCursorCurrentPosition;

			// Update Transform Widget
			MainWindow* mainWindow = m_globalInfo.mainWindow;
			mainWindow->infoPanel->entityInfoPanel->componentScrollArea->componentList->transformWidget->updateWidgets(m_entityMoveTransforms[m_globalInfo.currentEntityID]);
		}
	}
	event->accept();
}

void Renderer::wheelEvent(QWheelEvent* event) {
	if (!anyEntityTransformMode()) {
		m_mouseScrollY = static_cast<float>(event->angleDelta().y()) / 120.0f;
		event->accept();
	}
}

void Renderer::focusOutEvent(QFocusEvent* event) {
	m_cameraForwardKeyPressed = false;
	m_cameraBackwardKeyPressed = false;
	m_cameraLeftKeyPressed = false;
	m_cameraRightKeyPressed = false;
	m_cameraUpKeyPressed = false;
	m_cameraDownKeyPressed = false;
	m_moveCameraButtonPressed = false;
	setCursor(Qt::CursorShape::ArrowCursor);
	event->accept();
}

void Renderer::resizeEvent(QResizeEvent* event) {
	QOpenGLWidget::resizeEvent(event);
	m_gotResized = true;
}

void Renderer::dragEnterEvent(QDragEnterEvent* event) {
	if (event->mimeData()->hasUrls()) {
		event->acceptProposedAction();
	}
}

void Renderer::dragMoveEvent(QDragMoveEvent* event) {
	if (event->mimeData()->hasUrls()) {
		event->acceptProposedAction();
	}
}

void Renderer::dropEvent(QDropEvent* event) {
	QList<QUrl> sources = event->mimeData()->urls();
	if (!sources.isEmpty()) {
		std::string path = sources[0].toLocalFile().toStdString();
		std::string relativePath = AssetHelper::absoluteToRelative(path, m_globalInfo.projectDirectory);
		if (!path.empty()) {
			AssetHelper::FileType fileType = AssetHelper::fileType(path);
			if (fileType == AssetHelper::FileType::Model) { // Drag and drop model
				m_doPicking = true;
				m_dragDropResourceType = DragDropResourceType::Model;
				m_dragDropResourcePath = relativePath;
			}
			else if (fileType == AssetHelper::FileType::Material) { // Drag and drop material
				m_doPicking = true;
				m_dragDropResourceType = DragDropResourceType::Material;
				m_dragDropResourcePath = relativePath;
			}
		}
	}

	setFocus();
}

void Renderer::leaveEvent(QEvent* event) {
	(void)event;
	if (anyEntityTransformMode()) {
		QPoint localCursorPosition = mapFromGlobal(QCursor::pos());
		QPoint newCursorPosition = localCursorPosition;

		if (localCursorPosition.x() < 0) {
			newCursorPosition.setX(width() - 1);
			m_mouseCursorPreviousPosition.x = static_cast<float>(width());
		}
		else if (localCursorPosition.x() > (width() - 1)) {
			newCursorPosition.setX(0);
			m_mouseCursorPreviousPosition.x = -1.0f;
		}
		else {
			m_mouseCursorPreviousPosition.x = static_cast<float>(localCursorPosition.x());
		}
		if (localCursorPosition.y() < 0) {
			newCursorPosition.setY(height() - 1);
			m_mouseCursorPreviousPosition.y = 0.0f;
		}
		else if (localCursorPosition.y() > (height() - 1)) {
			newCursorPosition.setY(0);
			m_mouseCursorPreviousPosition.y = static_cast<float>(height() + 1);
		}
		else {
			m_mouseCursorPreviousPosition.y = static_cast<float>(height() - localCursorPosition.y());
		}
		QCursor::setPos(mapToGlobal(newCursorPosition));
	}
}
