#include "renderer.h"
#include "../undo_commands/destroy_entity_command.h"
#include "../undo_commands/change_entity_component_command.h"
#include "../../external/nlohmann/json.hpp"
#include <QKeySequence>
#include <QKeyEvent>
#include <fstream>
#include <array>
#include <cstdint>

Renderer::Renderer(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setFocusPolicy(Qt::FocusPolicy::ClickFocus);
	setMouseTracking(true);

	connect(&m_waitTimer, &QTimer::timeout, this, QOverload<>::of(&QWidget::update));
	connect(&globalInfo.signalEmitter, &SignalEmitter::toggleGridVisibilitySignal, this, &Renderer::onGridVisibilityToggled);
	connect(&globalInfo.signalEmitter, &SignalEmitter::toggleBackfaceCullingSignal, this, &Renderer::onBackfaceCullingToggled);
	connect(&globalInfo.signalEmitter, &SignalEmitter::toggleCamerasVisibilitySignal, this, &Renderer::onCamerasVisibilityToggled);
	connect(&globalInfo.signalEmitter, &SignalEmitter::toggleLightingSignal, this, &Renderer::onLightingToggled);
	connect(&globalInfo.signalEmitter, &SignalEmitter::toggleCollidersVisibilitySignal, this, &Renderer::onCollidersVisibilityToggled);
	connect(&globalInfo.signalEmitter, &SignalEmitter::switchCameraProjectionSignal, this, &Renderer::onCameraProjectionSwitched);
	connect(&globalInfo.signalEmitter, &SignalEmitter::resetCameraSignal, this, &Renderer::onCameraReset);
	connect(&globalInfo.signalEmitter, &SignalEmitter::orthographicCameraToAxisSignal, this, &Renderer::onOrthographicCameraToAxisChanged);

	std::fstream optionsFile("assets/options.json", std::ios::in);
	if (optionsFile.is_open()) {
		if (!nlohmann::json::accept(optionsFile)) {
			globalInfo.logger.addLog(LogLevel::Warning, "\"assets/options.json\" is not a valid JSON file.");
			return;
		}
	}
	else {
		globalInfo.logger.addLog(LogLevel::Warning, "\"assets/options.json\" cannot be opened.");
		return;
	}

	optionsFile = std::fstream("assets/options.json", std::ios::in);
	nlohmann::json j = nlohmann::json::parse(optionsFile);

	if (j.contains("renderer")) {
		if (j["renderer"].contains("cameraForwardKey")) {
			QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(j["renderer"]["cameraForwardKey"]));
			if (!sequence.isEmpty()) {
				m_cameraForwardKey = sequence[0].key();
			}
		}
		if (j["renderer"].contains("cameraBackwardKey")) {
			QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(j["renderer"]["cameraBackwardKey"]));
			if (!sequence.isEmpty()) {
				m_cameraBackwardKey = sequence[0].key();
			}
		}
		if (j["renderer"].contains("cameraLeftKey")) {
			QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(j["renderer"]["cameraLeftKey"]));
			if (!sequence.isEmpty()) {
				m_cameraLeftKey = sequence[0].key();
			}
		}
		if (j["renderer"].contains("cameraRightKey")) {
			QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(j["renderer"]["cameraRightKey"]));
			if (!sequence.isEmpty()) {
				m_cameraRightKey = sequence[0].key();
			}
		}
		if (j["renderer"].contains("cameraUpKey")) {
			QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(j["renderer"]["cameraUpKey"]));
			if (!sequence.isEmpty()) {
				m_cameraUpKey = sequence[0].key();
			}
		}
		if (j["renderer"].contains("cameraDownKey")) {
			QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(j["renderer"]["cameraDownKey"]));
			if (!sequence.isEmpty()) {
				m_cameraDownKey = sequence[0].key();
			}
		}
		if (j["renderer"].contains("translateEntityKey")) {
			QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(j["renderer"]["translateEntityKey"]));
			if (!sequence.isEmpty()) {
				m_translateEntityKey = sequence[0].key();
			}
		}
		if (j["renderer"].contains("rotateEntityKey")) {
			QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(j["renderer"]["rotateEntityKey"]));
			if (!sequence.isEmpty()) {
				m_rotateEntityKey = sequence[0].key();
			}
		}
		if (j["renderer"].contains("scaleEntityKey")) {
			QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(j["renderer"]["scaleEntityKey"]));
			if (!sequence.isEmpty()) {
				m_scaleEntityKey = sequence[0].key();
			}
		}
	}
}

Renderer::~Renderer() {
	gl.glDeleteBuffers(1, &m_globalInfo.rendererResourceManager.models["defaultCube"].meshes[0].vertexBuffer);
	gl.glDeleteBuffers(1, &m_globalInfo.rendererResourceManager.models["defaultCube"].meshes[0].indexBuffer);
	gl.glDeleteBuffers(1, &m_globalInfo.rendererResourceManager.models["cameraFrustumCube"].meshes[0].vertexBuffer);
	gl.glDeleteBuffers(1, &m_globalInfo.rendererResourceManager.models["cameraFrustumCube"].meshes[0].indexBuffer);
	gl.glDeleteBuffers(1, &m_lightBuffer);

	for (const auto& model : m_globalInfo.rendererResourceManager.models) {
		if ((model.first == "defaultCube") || (model.first == "cameraFrustumCube")) {
			continue;
		}
		for (const auto& mesh : model.second.meshes) {
			gl.glDeleteBuffers(1, &mesh.vertexBuffer);
			gl.glDeleteBuffers(1, &mesh.indexBuffer);
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

	struct Light {
		vec3 position;
		vec3 direction;
		vec3 color;
		vec2 cutoff;
	};

	in vec3 fragPosition;
	in vec3 fragNormal;
	in vec2 fragUV;

	uniform sampler2D diffuseTextureSampler;
	uniform sampler2D emissiveTextureSampler;
	uniform float alphaCutoff;
	uniform bool enableShading;
	restrict readonly buffer LightBuffer {
		uvec3 count;
		Light info[];
	} lights;

	out vec4 outColor;

	void main() {
		vec4 diffuseTextureSample = texture(diffuseTextureSampler, fragUV);
		if (diffuseTextureSample.a < alphaCutoff) {
			discard;
		}
		vec3 emissiveTextureSample = texture(emissiveTextureSampler, fragUV).rgb;
		outColor = vec4(0.0, 0.0, 0.0, 1.0);

		if (enableShading) {
			uint lightIndex = 0;

			// Directional Lights
			for (uint i = 0; i < lights.count.x; i++) {
				vec3 l = -lights.info[lightIndex].direction;

				outColor += vec4(diffuseTextureSample.rgb * lights.info[lightIndex].color * dot(l, fragNormal), 0.0);

				lightIndex++;
			}

			// Point Lights
			for (uint i = 0; i < lights.count.y; i++) {
				vec3 l = normalize(lights.info[lightIndex].position - fragPosition);
				float distance = length(lights.info[lightIndex].position - fragPosition);
				float attenuation = 1.0 / (distance * distance);
				vec3 radiance = lights.info[lightIndex].color * attenuation;

				outColor += vec4(diffuseTextureSample.rgb * radiance * dot(l, fragNormal), 0.0);

				lightIndex++;
			}

			// Spot Lights
			for (uint i = 0; i < lights.count.z; i++) {
				vec3 l = normalize(lights.info[lightIndex].position - fragPosition);
				float theta = dot(l, -lights.info[lightIndex].direction);
				float epsilon = cos(lights.info[lightIndex].cutoff.y) - cos(lights.info[lightIndex].cutoff.x);
				float intensity = clamp((theta - cos(lights.info[lightIndex].cutoff.x)) / epsilon, 0.0, 1.0);
				intensity = 1.0 - intensity;
				vec3 radiance = lights.info[lightIndex].color * intensity;

				outColor += vec4(diffuseTextureSample.rgb * radiance * dot(l, fragNormal), 0.0);

				lightIndex++;
			}
		}
		else {
			outColor = vec4(diffuseTextureSample.rgb, 1.0);
		}

		outColor += vec4(emissiveTextureSample, 0.0);
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

		outColor = grid(fragPos, 10.0) * float(t > 0.0);
		outColor.a *= fading;
	}
	)GLSL";
	GLuint gridFragmentShader = compileShader(GL_FRAGMENT_SHADER, gridFragmentShaderCode);

	m_gridProgram = compileProgram(gridVertexShader, gridFragmentShader);

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
				outColor = vec4(1.0, 1.0, 0.0, 1.0);
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

	out vec4 outColor;

	void main() {
		outColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
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
	std::vector<RendererResourceManager::MeshToGPU::Vertex> defaultCubeVertices = { { { 0.05f, 0.05f, -0.05f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f } }, { { -0.05f, 0.05f, -0.05f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f } }, { { -0.05f, 0.05f, 0.05f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f } }, { { 0.05f, 0.05f, 0.05f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } }, { { 0.05f, -0.05f, 0.05f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f } }, { { 0.05f, 0.05f, 0.05f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } }, { { -0.05f, 0.05f, 0.05f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } }, { { -0.05f, -0.05f, 0.05f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } }, { { -0.05f, -0.05f, 0.05f }, { -1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } }, { { -0.05f, 0.05f, 0.05f }, { -1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f } }, { { -0.05f, 0.05f, -0.05f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f } }, { { -0.05f, -0.05f, -0.05f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } }, { { -0.05f, -0.05f, -0.05f }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 0.0f } }, { { 0.05f, -0.05f, -0.05f }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 1.0f } }, { { 0.05f, -0.05f, 0.05f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 1.0f } }, { { -0.05f, -0.05f, 0.05f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f } }, { { 0.05f, -0.05f, -0.05f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } }, { { 0.05f, 0.05f, -0.05f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f } }, { { 0.05f, 0.05f, 0.05f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f } }, { { 0.05f, -0.05f, 0.05f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } }, { { -0.05f, -0.05f, -0.05f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 0.0f } }, { { -0.05f, 0.05f, -0.05f}, { 0.0f, 0.0f, -1.0f }, { 1.0f, 1.0f } }, { { 0.05f, 0.05f, -0.05f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f } }, { { 0.05f, -0.05f, -0.05f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f } } };
	gl.glBindBuffer(GL_ARRAY_BUFFER, defaultCubeVertexBuffer);
	gl.glBufferData(GL_ARRAY_BUFFER, defaultCubeVertices.size() * sizeof(RendererResourceManager::MeshToGPU::Vertex), defaultCubeVertices.data(), GL_STATIC_DRAW);

	RendererMesh defaultCubeMesh;
	defaultCubeMesh.vertexBuffer = defaultCubeVertexBuffer;
	defaultCubeMesh.indexBuffer = cubeTriangleIndexBuffer;
	defaultCubeMesh.indexCount = static_cast<GLuint>(cubeTriangleIndices.size());
	defaultCubeMesh.diffuseTexturePath = "defaultDiffuseTexture";
	defaultCubeMesh.emissiveTexturePath = "defaultEmissiveTexture";

	RendererModel defaultCubeModel;
	defaultCubeModel.meshes.push_back(defaultCubeMesh);

	m_globalInfo.rendererResourceManager.models["defaultCube"] = defaultCubeModel;

	// Default textures
	GLuint defaultDiffuseTexture;
	gl.glGenTextures(1, &defaultDiffuseTexture);
	std::vector<uint8_t> diffuseTextureData = { 145, 99, 65, 255, 208, 194, 175, 255, 208, 194, 175, 255, 145, 99, 65, 255 };
	gl.glBindTexture(GL_TEXTURE_2D, defaultDiffuseTexture);
	gl.glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, diffuseTextureData.data());
	gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	m_globalInfo.rendererResourceManager.textures["defaultDiffuseTexture"] = defaultDiffuseTexture;

	GLuint defaultEmissiveTexture;
	gl.glGenTextures(1, &defaultEmissiveTexture);
	std::vector<uint8_t> emissiveTextureData = { 0, 0, 0, 255 };
	gl.glBindTexture(GL_TEXTURE_2D, defaultEmissiveTexture);
	gl.glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, emissiveTextureData.data());
	gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	m_globalInfo.rendererResourceManager.textures["defaultEmissiveTexture"] = defaultEmissiveTexture;

	// Frustum cube
	GLuint cameraFrustumCubeVertexBuffer;
	gl.glGenBuffers(1, &cameraFrustumCubeVertexBuffer);
	std::vector<RendererResourceManager::MeshToGPU::Vertex> cameraFrustumCubeVertices = { { { -1.0f, -1.0f, -1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } }, { { 1.0f, -1.0f, -1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } }, { { 1.0f, -1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } }, { { -1.0f, -1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } }, { { -1.0f, 1.0f, -1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } }, { { 1.0f, 1.0f, -1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } }, { { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } }, { { -1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } } };
	gl.glBindBuffer(GL_ARRAY_BUFFER, cameraFrustumCubeVertexBuffer);
	gl.glBufferData(GL_ARRAY_BUFFER, cameraFrustumCubeVertices.size() * sizeof(RendererResourceManager::MeshToGPU::Vertex), cameraFrustumCubeVertices.data(), GL_STATIC_DRAW);

	RendererMesh cameraFrustumCubeMesh;
	cameraFrustumCubeMesh.vertexBuffer = cameraFrustumCubeVertexBuffer;
	cameraFrustumCubeMesh.indexBuffer = cubeLineIndexBuffer;
	cameraFrustumCubeMesh.indexCount = static_cast<GLuint>(cubeLineIndices.size());
	cameraFrustumCubeMesh.diffuseTexturePath = "defaultDiffuseTexture";
	cameraFrustumCubeMesh.emissiveTexturePath = "defaultEmissiveTexture";

	RendererModel cameraFrustumCubeModel;
	cameraFrustumCubeModel.meshes.push_back(cameraFrustumCubeMesh);

	m_globalInfo.rendererResourceManager.models["cameraFrustumCube"] = cameraFrustumCubeModel;

	// Light
	createLightBuffer();

	// Start render
	m_waitTimer.setInterval(16);
	m_waitTimer.start();
}

void Renderer::paintGL() {
	if (m_gotResized) {
		createPickingImages();
		createOutlineSoloImages();

		m_gotResized = false;
	}

	loadResourcesToGPU();

	updateCamera();

	if (m_lightingEnabled) {
		updateLights();
	}

	if (m_backfaceCullingEnabled) {
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
			const Transform& transform = ((entity.second.entityID == m_globalInfo.currentEntityID) && m_entityMoveTransform) ? m_entityMoveTransform.value() : entity.second.transform;
			nml::mat4 rotationMatrix = nml::rotate(nml::toRad(transform.rotation.x), nml::vec3(1.0f, 0.0f, 0.0f)) * nml::rotate(nml::toRad(transform.rotation.y), nml::vec3(0.0f, 1.0f, 0.0f)) * nml::rotate(nml::toRad(transform.rotation.z), nml::vec3(0.0f, 0.0f, 1.0f));
			nml::mat4 modelMatrix = nml::translate(transform.position) * rotationMatrix * nml::scale(transform.scale);

			gl.glUniformMatrix4fv(gl.glGetUniformLocation(m_entityProgram, "model"), 1, false, modelMatrix.data());

			if (entity.second.renderable && (m_globalInfo.rendererResourceManager.models.find(entity.second.renderable->modelPath) != m_globalInfo.rendererResourceManager.models.end())) {
				const RendererModel& entityModel = m_globalInfo.rendererResourceManager.models[entity.second.renderable->modelPath];
				for (const auto& entityMesh : entityModel.meshes) {
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

					gl.glActiveTexture(GL_TEXTURE0);
					gl.glBindTexture(GL_TEXTURE_2D, m_globalInfo.rendererResourceManager.textures[entityMesh.diffuseTexturePath]);
					gl.glUniform1i(gl.glGetUniformLocation(m_entityProgram, "diffuseTextureSampler"), 0);

					gl.glActiveTexture(GL_TEXTURE1);
					gl.glBindTexture(GL_TEXTURE_2D, m_globalInfo.rendererResourceManager.textures[entityMesh.emissiveTexturePath]);
					gl.glUniform1i(gl.glGetUniformLocation(m_entityProgram, "emissiveTextureSampler"), 1);

					gl.glUniform1f(gl.glGetUniformLocation(m_entityProgram, "alphaCutoff"), entityMesh.alphaCutoff);

					gl.glUniform1i(gl.glGetUniformLocation(m_entityProgram, "enableShading"), m_lightingEnabled);

					gl.glDrawElements(GL_TRIANGLES, entityMesh.indexCount, GL_UNSIGNED_INT, NULL);
				}
			}
			else {
				RendererMesh& defaultMesh = m_globalInfo.rendererResourceManager.models["defaultCube"].meshes[0];
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

				gl.glActiveTexture(GL_TEXTURE0);
				gl.glBindTexture(GL_TEXTURE_2D, m_globalInfo.rendererResourceManager.textures[defaultMesh.diffuseTexturePath]);
				gl.glUniform1i(gl.glGetUniformLocation(m_entityProgram, "diffuseTextureSampler"), 0);

				gl.glActiveTexture(GL_TEXTURE1);
				gl.glBindTexture(GL_TEXTURE_2D, m_globalInfo.rendererResourceManager.textures[defaultMesh.emissiveTexturePath]);
				gl.glUniform1i(gl.glGetUniformLocation(m_entityProgram, "emissiveTextureSampler"), 1);

				gl.glUniform1i(gl.glGetUniformLocation(m_entityProgram, "enableShading"), 0);

				gl.glDrawElements(GL_TRIANGLES, defaultMesh.indexCount, GL_UNSIGNED_INT, NULL);
			}
		}
	}

	gl.glDisable(GL_CULL_FACE);
	
	// Entities Cameras
	if (m_showCameras) {
		gl.glUseProgram(m_cameraFrustumProgram);
		gl.glUniformMatrix4fv(gl.glGetUniformLocation(m_cameraFrustumProgram, "viewProj"), 1, false, m_camera.viewProjMatrix.data());

		gl.glBindBuffer(GL_ARRAY_BUFFER, m_globalInfo.rendererResourceManager.models["cameraFrustumCube"].meshes[0].vertexBuffer);
		GLint positionPos = gl.glGetAttribLocation(m_cameraFrustumProgram, "position");
		gl.glEnableVertexAttribArray(positionPos);
		gl.glVertexAttribPointer(positionPos, 3, GL_FLOAT, false, 32, (void*)0);
		gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_globalInfo.rendererResourceManager.models["cameraFrustumCube"].meshes[0].indexBuffer);

		for (const auto& entity : m_globalInfo.entities) {
			if (entity.second.isVisible) {
				if (entity.second.camera) {
					const Transform& transform = ((entity.second.entityID == m_globalInfo.currentEntityID) && m_entityMoveTransform) ? m_entityMoveTransform.value() : entity.second.transform;
					nml::mat4 entityCameraViewMatrix = nml::lookAtRH(transform.position, transform.position + entity.second.camera->forward, entity.second.camera->up);
					nml::mat4 entityCameraRotation = nml::rotate(nml::toRad(transform.rotation.x), nml::vec3(1.0f, 0.0f, 0.0f)) * nml::rotate(nml::toRad(transform.rotation.y), nml::vec3(0.0f, 1.0f, 0.0f)) * nml::rotate(nml::toRad(transform.rotation.z), nml::vec3(0.0f, 0.0f, 1.0f));
					nml::mat4 entityCameraProjectionMatrix = nml::perspectiveRH(nml::toRad(entity.second.camera->fov), 16.0f / 9.0f, entity.second.camera->nearPlane, entity.second.camera->farPlane);
					nml::mat4 invEntityCameraModel = nml::inverse(entityCameraProjectionMatrix * entityCameraRotation * entityCameraViewMatrix);
					gl.glUniformMatrix4fv(gl.glGetUniformLocation(m_cameraFrustumProgram, "model"), 1, false, invEntityCameraModel.data());

					gl.glDrawElements(GL_LINES, m_globalInfo.rendererResourceManager.models["cameraFrustumCube"].meshes[0].indexCount, GL_UNSIGNED_INT, NULL);
				}
			}
		}
	}

	// Entities Colliders
	if (m_showColliders) {
		gl.glUseProgram(m_colliderProgram);
		gl.glUniformMatrix4fv(gl.glGetUniformLocation(m_colliderProgram, "viewProj"), 1, false, m_camera.viewProjMatrix.data());

		for (const auto& entity : m_globalInfo.entities) {
			if (entity.second.isVisible) {
				if (entity.second.collidable && (m_globalInfo.rendererResourceManager.models.find("Collider_" + std::to_string(entity.first)) != m_globalInfo.rendererResourceManager.models.end())) {
					const Transform& transform = ((entity.second.entityID == m_globalInfo.currentEntityID) && m_entityMoveTransform) ? m_entityMoveTransform.value() : entity.second.transform;
					nml::mat4 rotationMatrix = nml::rotate(nml::toRad(transform.rotation.x), nml::vec3(1.0f, 0.0f, 0.0f)) * nml::rotate(nml::toRad(transform.rotation.y), nml::vec3(0.0f, 1.0f, 0.0f)) * nml::rotate(nml::toRad(transform.rotation.z), nml::vec3(0.0f, 0.0f, 1.0f));
					nml::mat4 modelMatrix = nml::translate(transform.position) * rotationMatrix * nml::scale(transform.scale);

					gl.glUniformMatrix4fv(gl.glGetUniformLocation(m_colliderProgram, "model"), 1, false, modelMatrix.data());

					const RendererModel& colliderModel = m_globalInfo.rendererResourceManager.models["Collider_" + std::to_string(entity.first)];
					for (const auto& colliderMesh : colliderModel.meshes) {
						gl.glBindBuffer(GL_ARRAY_BUFFER, colliderMesh.vertexBuffer);
						GLint positionPos = gl.glGetAttribLocation(m_colliderProgram, "position");
						gl.glEnableVertexAttribArray(positionPos);
						gl.glVertexAttribPointer(positionPos, 3, GL_FLOAT, false, 32, (void*)0);
						gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, colliderMesh.indexBuffer);

						gl.glDrawElements(GL_LINES, colliderMesh.indexCount, GL_UNSIGNED_INT, NULL);
					}
				}
			}
		}
	}

	// Grid
	if (m_showGrid) {
		if (!m_camera.useOrthographicProjection) {
			gl.glUseProgram(m_gridProgram);
			gl.glUniformMatrix4fv(gl.glGetUniformLocation(m_gridProgram, "view"), 1, false, m_camera.viewMatrix.data());
			gl.glUniformMatrix4fv(gl.glGetUniformLocation(m_gridProgram, "projection"), 1, false, m_camera.projectionMatrix.data());
			gl.glUniformMatrix4fv(gl.glGetUniformLocation(m_gridProgram, "viewProj"), 1, false, m_camera.viewProjMatrix.data());
			gl.glUniform1f(gl.glGetUniformLocation(m_gridProgram, "near"), m_camera.nearPlane);
			gl.glUniform1f(gl.glGetUniformLocation(m_gridProgram, "far"), m_camera.farPlane);

			gl.glDrawArrays(GL_TRIANGLES, 0, 6);
		}
	}

	if (m_backfaceCullingEnabled) {
		gl.glEnable(GL_CULL_FACE);
	}
	else {
		gl.glDisable(GL_CULL_FACE);
	}

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
				const Transform& transform = ((entity.second.entityID == m_globalInfo.currentEntityID) && m_entityMoveTransform) ? m_entityMoveTransform.value() : entity.second.transform;
				nml::mat4 rotationMatrix = nml::rotate(nml::toRad(transform.rotation.x), nml::vec3(1.0f, 0.0f, 0.0f)) * nml::rotate(nml::toRad(transform.rotation.y), nml::vec3(0.0f, 1.0f, 0.0f)) * nml::rotate(nml::toRad(transform.rotation.z), nml::vec3(0.0f, 0.0f, 1.0f));
				nml::mat4 modelMatrix = nml::translate(transform.position) * rotationMatrix * nml::scale(transform.scale);
				gl.glUniformMatrix4fv(gl.glGetUniformLocation(m_pickingProgram, "model"), 1, false, modelMatrix.data());

				glex.glUniform1ui(gl.glGetUniformLocation(m_pickingProgram, "entityID"), entity.second.entityID);

				if (entity.second.renderable && (m_globalInfo.rendererResourceManager.models.find(entity.second.renderable->modelPath) != m_globalInfo.rendererResourceManager.models.end())) {
					const RendererModel& entityModel = m_globalInfo.rendererResourceManager.models[entity.second.renderable->modelPath];
					for (const auto& entityMesh : entityModel.meshes) {
						gl.glBindBuffer(GL_ARRAY_BUFFER, entityMesh.vertexBuffer);
						GLint positionPos = gl.glGetAttribLocation(m_pickingProgram, "position");
						gl.glEnableVertexAttribArray(positionPos);
						gl.glVertexAttribPointer(positionPos, 3, GL_FLOAT, false, 32, (void*)0);
						gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, entityMesh.indexBuffer);

						gl.glDrawElements(GL_TRIANGLES, entityMesh.indexCount, GL_UNSIGNED_INT, NULL);
					}
				}
				else {
					RendererMesh& defaultMesh = m_globalInfo.rendererResourceManager.models["defaultCube"].meshes[0];
					gl.glBindBuffer(GL_ARRAY_BUFFER, defaultMesh.vertexBuffer);
					GLint positionPos = gl.glGetAttribLocation(m_pickingProgram, "position");
					gl.glEnableVertexAttribArray(positionPos);
					gl.glVertexAttribPointer(positionPos, 3, GL_FLOAT, false, 32, (void*)0);
					gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, defaultMesh.indexBuffer);

					gl.glDrawElements(GL_TRIANGLES, defaultMesh.indexCount, GL_UNSIGNED_INT, NULL);
				}
			}
		}

		QPoint cursorPosition = mapFromGlobal(QCursor::pos());
		uint32_t pickedEntityID;
		gl.glReadPixels(cursorPosition.x() * m_globalInfo.devicePixelRatio, (height() - cursorPosition.y()) * m_globalInfo.devicePixelRatio, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, &pickedEntityID);
		if (pickedEntityID != NO_ENTITY) {
			m_globalInfo.currentEntityID = pickedEntityID;
		}
		else {
			m_globalInfo.currentEntityID = NO_ENTITY;
		}
		emit m_globalInfo.signalEmitter.selectEntitySignal();

		m_doPicking = false;
	}

	gl.glDisable(GL_CULL_FACE);

	// Outline
	if (m_globalInfo.currentEntityID != NO_ENTITY) {
		const Entity& entity = m_globalInfo.entities[m_globalInfo.currentEntityID];

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
		const Transform& transform = ((entity.entityID == m_globalInfo.currentEntityID) && m_entityMoveTransform) ? m_entityMoveTransform.value() : entity.transform;
		nml::mat4 rotationMatrix = nml::rotate(nml::toRad(transform.rotation.x), nml::vec3(1.0f, 0.0f, 0.0f)) * nml::rotate(nml::toRad(transform.rotation.y), nml::vec3(0.0f, 1.0f, 0.0f)) * nml::rotate(nml::toRad(transform.rotation.z), nml::vec3(0.0f, 0.0f, 1.0f));
		nml::mat4 modelMatrix = nml::translate(transform.position) * rotationMatrix * nml::scale(transform.scale);
		gl.glUniformMatrix4fv(gl.glGetUniformLocation(m_outlineSoloProgram, "model"), 1, false, modelMatrix.data());

		if (entity.renderable && (m_globalInfo.rendererResourceManager.models.find(entity.renderable->modelPath) != m_globalInfo.rendererResourceManager.models.end())) {
			const RendererModel& entityModel = m_globalInfo.rendererResourceManager.models[entity.renderable->modelPath];
			for (const auto& entityMesh : entityModel.meshes) {
				gl.glBindBuffer(GL_ARRAY_BUFFER, entityMesh.vertexBuffer);
				GLint positionPos = gl.glGetAttribLocation(m_outlineSoloProgram, "position");
				gl.glEnableVertexAttribArray(positionPos);
				gl.glVertexAttribPointer(positionPos, 3, GL_FLOAT, false, 32, (void*)0);
				gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, entityMesh.indexBuffer);

				gl.glDrawElements(GL_TRIANGLES, entityMesh.indexCount, GL_UNSIGNED_INT, NULL);
			}
		}
		else {
			RendererMesh& defaultMesh = m_globalInfo.rendererResourceManager.models["defaultCube"].meshes[0];
			gl.glBindBuffer(GL_ARRAY_BUFFER, defaultMesh.vertexBuffer);
			GLint positionPos = gl.glGetAttribLocation(m_outlineSoloProgram, "position");
			gl.glEnableVertexAttribArray(positionPos);
			gl.glVertexAttribPointer(positionPos, 3, GL_FLOAT, false, 32, (void*)0);
			gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, defaultMesh.indexBuffer);

			gl.glDrawElements(GL_TRIANGLES, defaultMesh.indexCount, GL_UNSIGNED_INT, NULL);
		}

		// Entity Camera
		if (m_showCameras) {
			if (entity.camera) {
				nml::mat4 entityCameraViewMatrix;
				nml::mat4 entityCameraRotation;
				if ((entity.entityID == m_globalInfo.currentEntityID) && m_entityMoveTransform) {
					entityCameraViewMatrix = nml::lookAtRH(m_entityMoveTransform->position, m_entityMoveTransform->position + entity.camera->forward, entity.camera->up);
					entityCameraRotation = nml::rotate(nml::toRad(m_entityMoveTransform->rotation.x), nml::vec3(1.0f, 0.0f, 0.0f)) * nml::rotate(nml::toRad(m_entityMoveTransform->rotation.y), nml::vec3(0.0f, 1.0f, 0.0f)) * nml::rotate(nml::toRad(m_entityMoveTransform->rotation.z), nml::vec3(0.0f, 0.0f, 1.0f));
				}
				else {
					entityCameraViewMatrix = nml::lookAtRH(entity.transform.position, entity.transform.position + entity.camera->forward, entity.camera->up);
					entityCameraRotation = nml::rotate(nml::toRad(entity.transform.rotation.x), nml::vec3(1.0f, 0.0f, 0.0f)) * nml::rotate(nml::toRad(entity.transform.rotation.y), nml::vec3(0.0f, 1.0f, 0.0f)) * nml::rotate(nml::toRad(entity.transform.rotation.z), nml::vec3(0.0f, 0.0f, 1.0f));
				}
				nml::mat4 entityCameraProjectionMatrix = nml::perspectiveRH(nml::toRad(entity.camera->fov), 16.0f / 9.0f, entity.camera->nearPlane, entity.camera->farPlane);
				nml::mat4 invEntityCameraModel = nml::inverse(entityCameraProjectionMatrix * entityCameraRotation * entityCameraViewMatrix);
				gl.glUniformMatrix4fv(gl.glGetUniformLocation(m_outlineSoloProgram, "model"), 1, false, invEntityCameraModel.data());

				gl.glBindBuffer(GL_ARRAY_BUFFER, m_globalInfo.rendererResourceManager.models["cameraFrustumCube"].meshes[0].vertexBuffer);
				GLint positionPos = gl.glGetAttribLocation(m_outlineSoloProgram, "position");
				gl.glEnableVertexAttribArray(positionPos);
				gl.glVertexAttribPointer(positionPos, 3, GL_FLOAT, false, 32, (void*)0);
				gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_globalInfo.rendererResourceManager.models["cameraFrustumCube"].meshes[0].indexBuffer);

				gl.glDrawElements(GL_LINES, m_globalInfo.rendererResourceManager.models["cameraFrustumCube"].meshes[0].indexCount, GL_UNSIGNED_INT, NULL);
			}
		}

		// Entity Collider
		if (m_showColliders) {
			if (entity.isVisible) {
				if (entity.collidable && (m_globalInfo.rendererResourceManager.models.find("Collider_" + std::to_string(entity.entityID)) != m_globalInfo.rendererResourceManager.models.end())) {
					const RendererModel& colliderModel = m_globalInfo.rendererResourceManager.models["Collider_" + std::to_string(entity.entityID)];
					gl.glUniformMatrix4fv(gl.glGetUniformLocation(m_outlineSoloProgram, "model"), 1, false, modelMatrix.data());
					for (const auto& colliderMesh : colliderModel.meshes) {
						gl.glBindBuffer(GL_ARRAY_BUFFER, colliderMesh.vertexBuffer);
						GLint positionPos = gl.glGetAttribLocation(m_outlineSoloProgram, "position");
						gl.glEnableVertexAttribArray(positionPos);
						gl.glVertexAttribPointer(positionPos, 3, GL_FLOAT, false, 32, (void*)0);
						gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, colliderMesh.indexBuffer);

						gl.glDrawElements(GL_LINES, colliderMesh.indexCount, GL_UNSIGNED_INT, NULL);
					}
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

		gl.glBindBuffer(GL_ARRAY_BUFFER, 0);

		gl.glDrawArrays(GL_TRIANGLES, 0, 3);
	}
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
		std::vector<GLchar> infolog;
		infolog.reserve(maxlength);
		int length;
		gl.glGetShaderInfoLog(shader, maxlength, &length, infolog.data());
		m_globalInfo.logger.addLog(LogLevel::Error, "Error while compiling shader :" + std::string(infolog.data(), length));

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
		std::vector<GLchar> infolog;
		int length;
		gl.glGetProgramInfoLog(program, maxlength, &length, infolog.data());
		m_globalInfo.logger.addLog(LogLevel::Error, "Error while linking program :" + std::string(infolog.data(), length));
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

void Renderer::createLightBuffer() {
	gl.glGenBuffers(1, &m_lightBuffer);
	gl.glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_lightBuffer);
	gl.glBufferData(GL_SHADER_STORAGE_BUFFER, 32768, NULL, GL_DYNAMIC_DRAW);
}

bool Renderer::anyEntityTransformKeyPressed() {
	return m_translateEntityKeyPressed || m_rotateEntityKeyPressed || m_scaleEntityKeyPressed;
}

void Renderer::updateCamera() {
	if (anyEntityTransformKeyPressed()) {
		return;
	}

	float deltaTime = static_cast<float>(m_waitTimer.interval()) / 1000.0f;

	if (!m_camera.useOrthographicProjection) {
		m_camera.perspectiveYaw = std::fmod(m_camera.perspectiveYaw + m_mouseCursorDifference.x, 360.0f);
		m_camera.perspectivePitch = std::max(-89.0f, std::min(89.0f, m_camera.perspectivePitch + m_mouseCursorDifference.y));

		float cameraYawRad = nml::toRad(m_camera.perspectiveYaw);
		float cameraPitchRad = nml::toRad(m_camera.perspectivePitch);

		m_camera.perspectiveDirection.x = std::cos(cameraPitchRad) * std::cos(cameraYawRad);
		m_camera.perspectiveDirection.y = -std::sin(cameraPitchRad);
		m_camera.perspectiveDirection.z = std::cos(cameraPitchRad) * std::sin(cameraYawRad);
		m_camera.perspectiveDirection = nml::normalize(m_camera.perspectiveDirection);

		nml::vec3 t = nml::normalize(nml::vec3(-m_camera.perspectiveDirection.z, 0.0f, m_camera.perspectiveDirection.x));

		float cameraSpeed = m_camera.speed * deltaTime;

		if (m_cameraForwardKeyPressed) {
			m_camera.perspectivePosition += m_camera.perspectiveDirection * cameraSpeed;
		}
		if (m_cameraBackwardKeyPressed) {
			m_camera.perspectivePosition -= m_camera.perspectiveDirection * cameraSpeed;
		}
		if (m_cameraLeftKeyPressed) {
			m_camera.perspectivePosition -= t * cameraSpeed;
		}
		if (m_cameraRightKeyPressed) {
			m_camera.perspectivePosition += t * cameraSpeed;
		}
		if (m_cameraUpKeyPressed) {
			m_camera.perspectivePosition.y += cameraSpeed;
		}
		if (m_cameraDownKeyPressed) {
			m_camera.perspectivePosition.y -= cameraSpeed;
		}

		m_camera.viewMatrix = nml::lookAtRH(m_camera.perspectivePosition, m_camera.perspectivePosition + m_camera.perspectiveDirection, m_camera.perspectiveUp);
		m_camera.projectionMatrix = nml::perspectiveRH(nml::toRad(45.0f), static_cast<float>(width()) / static_cast<float>(height()), m_camera.nearPlane, m_camera.farPlane);
	}
	else {
		nml::vec3 t;
		if ((m_camera.orthographicDirection.y == 1.0f) || (m_camera.orthographicDirection.y == -1.0f)) {
			t = nml::normalize(nml::vec3(-m_camera.orthographicDirection.y, 0.0f, m_camera.orthographicDirection.x));
		}
		else {
			t = nml::normalize(nml::vec3(-m_camera.orthographicDirection.z, 0.0f, m_camera.orthographicDirection.x));
		}

		float horizontalSpeed = m_camera.orthographicHalfExtent * m_camera.speed * deltaTime * ((m_mouseCursorDifference.x == 0.0f) ? 1.0f : std::abs(m_mouseCursorDifference.x));
		float verticalSpeed = m_camera.orthographicHalfExtent * m_camera.speed * deltaTime * ((m_mouseCursorDifference.y == 0.0f) ? 1.0f : std::abs(m_mouseCursorDifference.y));
		float halfExtentSpeed = m_camera.speed * 5.0f * ((m_mouseScrollY == 0.0f) ? 1.0f : 2.0f) * deltaTime;

		if (m_cameraForwardKeyPressed || (m_mouseCursorDifference.y < 0.0f)) {
			m_camera.orthographicPosition += m_camera.orthographicUp * verticalSpeed;
		}
		if (m_cameraBackwardKeyPressed || (m_mouseCursorDifference.y > 0.0f)) {
			m_camera.orthographicPosition -= m_camera.orthographicUp * verticalSpeed;
		}
		if (m_cameraLeftKeyPressed || (m_mouseCursorDifference.x < 0.0f)) {
			m_camera.orthographicPosition -= t * horizontalSpeed;
		}
		if (m_cameraRightKeyPressed || (m_mouseCursorDifference.x > 0.0f)) {
			m_camera.orthographicPosition += t * horizontalSpeed;
		}
		if (m_cameraUpKeyPressed || (m_mouseScrollY < 0.0f)) {
			m_camera.orthographicHalfExtent += halfExtentSpeed;
		}
		if (m_cameraDownKeyPressed || (m_mouseScrollY > 0.0f)) {
			m_camera.orthographicHalfExtent -= halfExtentSpeed;
			m_camera.orthographicHalfExtent = std::max(m_camera.orthographicHalfExtent, 0.01f);
		}

		m_camera.viewMatrix = nml::lookAtRH(m_camera.orthographicPosition, m_camera.orthographicPosition + m_camera.orthographicDirection, m_camera.orthographicUp);
		float orthographicHalfExtentWidth = m_camera.orthographicHalfExtent * static_cast<float>(width()) / static_cast<float>(height());
		m_camera.projectionMatrix = nml::orthoRH(-orthographicHalfExtentWidth, orthographicHalfExtentWidth, -m_camera.orthographicHalfExtent, m_camera.orthographicHalfExtent, m_camera.nearPlane, m_camera.farPlane);
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

	for (const auto& entity : m_globalInfo.entities) {
		if (entity.second.light) {
			const Transform& transform = ((entity.second.entityID == m_globalInfo.currentEntityID) && m_entityMoveTransform) ? m_entityMoveTransform.value() : entity.second.transform;
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
				directionalLightsInfos.push_back(nml::vec4(light.color, 0.0f));
				directionalLightsInfos.push_back(nml::vec4());
			}
			else if (light.type == "Point") {
				lightsCount[1]++;

				pointLightsInfos.push_back(nml::vec4(transform.position, 0.0f));
				pointLightsInfos.push_back(nml::vec4());
				pointLightsInfos.push_back(nml::vec4(light.color, 0.0f));
				pointLightsInfos.push_back(nml::vec4());
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
				spotLightsInfos.push_back(nml::vec4(light.color, 0.0f));
				spotLightsInfos.push_back(nml::vec4(nml::toRad(light.cutoff.x), nml::toRad(light.cutoff.y), 0.0f, 0.0f));
			}
		}
	}

	gl.glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 4 * sizeof(uint32_t), lightsCount.data());
	gl.glBufferSubData(GL_SHADER_STORAGE_BUFFER, 4 * sizeof(uint32_t), directionalLightsInfos.size() * sizeof(nml::vec4), directionalLightsInfos.data());
	gl.glBufferSubData(GL_SHADER_STORAGE_BUFFER, 4 * sizeof(uint32_t) + directionalLightsInfos.size() * sizeof(nml::vec4), pointLightsInfos.size() * sizeof(nml::vec4), pointLightsInfos.data());
	gl.glBufferSubData(GL_SHADER_STORAGE_BUFFER, 4 * sizeof(uint32_t) + directionalLightsInfos.size() * sizeof(nml::vec4) + pointLightsInfos.size() * sizeof(nml::vec4), spotLightsInfos.size() * sizeof(nml::vec4), spotLightsInfos.data());
}

void Renderer::loadResourcesToGPU() {
	for (const auto& modelToGPU : m_globalInfo.rendererResourceManager.modelsToGPU) {
		if (m_globalInfo.rendererResourceManager.models.find(modelToGPU.first) != m_globalInfo.rendererResourceManager.models.end()) {
			// Free resources for same model
			for (auto& mesh : m_globalInfo.rendererResourceManager.models[modelToGPU.first].meshes) {
				gl.glDeleteBuffers(1, &mesh.vertexBuffer);
				gl.glDeleteBuffers(1, &mesh.indexBuffer);
			}
		}

		RendererModel newRendererModel;
		for (const auto& meshToGPU : modelToGPU.second.meshes) {
			RendererMesh newRendererMesh;
			gl.glGenBuffers(1, &newRendererMesh.vertexBuffer);
			gl.glBindBuffer(GL_ARRAY_BUFFER, newRendererMesh.vertexBuffer);
			gl.glBufferData(GL_ARRAY_BUFFER, meshToGPU.vertices.size() * sizeof(RendererResourceManager::MeshToGPU::Vertex), meshToGPU.vertices.data(), GL_STATIC_DRAW);
			
			gl.glGenBuffers(1, &newRendererMesh.indexBuffer);
			gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, newRendererMesh.indexBuffer);
			gl.glBufferData(GL_ELEMENT_ARRAY_BUFFER, meshToGPU.indices.size() * sizeof(uint32_t), meshToGPU.indices.data(), GL_STATIC_DRAW);

			newRendererMesh.indexCount = static_cast<uint32_t>(meshToGPU.indices.size());

			newRendererMesh.diffuseTexturePath = meshToGPU.diffuseTexturePath;
			newRendererMesh.emissiveTexturePath = meshToGPU.emissiveTexturePath;

			newRendererMesh.alphaCutoff = meshToGPU.alphaCutoff;

			newRendererMesh.obb = meshToGPU.obb;
			newRendererMesh.sphere = meshToGPU.sphere;
			newRendererMesh.capsule = meshToGPU.capsule;

			newRendererModel.meshes.push_back(newRendererMesh);
		}

		m_globalInfo.rendererResourceManager.models[modelToGPU.first] = newRendererModel;
	}
	m_globalInfo.rendererResourceManager.modelsToGPU.clear();

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
		if (imageToGPU.second.minFilter == RendererResourceManager::ImageToGPU::SamplerFilter::Nearest) {
			if (imageToGPU.second.mipmapFilter == RendererResourceManager::ImageToGPU::SamplerFilter::Nearest) {
				gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
			}
			else {
				gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
			}
		}
		else if (imageToGPU.second.minFilter == RendererResourceManager::ImageToGPU::SamplerFilter::Linear) {
			if (imageToGPU.second.mipmapFilter == RendererResourceManager::ImageToGPU::SamplerFilter::Nearest) {
				gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
			}
			else {
				gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			}
		}

		if (imageToGPU.second.magFilter == RendererResourceManager::ImageToGPU::SamplerFilter::Nearest) {
			gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}
		else if (imageToGPU.second.magFilter == RendererResourceManager::ImageToGPU::SamplerFilter::Linear) {
			gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}

		if (imageToGPU.second.wrapS == RendererResourceManager::ImageToGPU::SamplerWrap::ClampToEdge) {
			gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		}
		else if (imageToGPU.second.wrapS == RendererResourceManager::ImageToGPU::SamplerWrap::Repeat) {
			gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		}
		else if (imageToGPU.second.wrapS == RendererResourceManager::ImageToGPU::SamplerWrap::MirroredRepeat) {
			gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		}

		if (imageToGPU.second.wrapT == RendererResourceManager::ImageToGPU::SamplerWrap::ClampToEdge) {
			gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}
		else if (imageToGPU.second.wrapT == RendererResourceManager::ImageToGPU::SamplerWrap::Repeat) {
			gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}
		else if (imageToGPU.second.wrapT == RendererResourceManager::ImageToGPU::SamplerWrap::MirroredRepeat) {
			gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
		}
	}
	m_globalInfo.rendererResourceManager.imagesToGPU.clear();
}

nml::vec3 Renderer::unproject(const nml::vec2& p, float width, float height, const nml::mat4& invViewMatrix, const nml::mat4& invProjMatrix) {
	nml::vec2 screenSpace = nml::vec2(p.x / width, p.y / height);
	nml::vec2 clipSpace = (screenSpace * 2.0f) - 1.0f;
	nml::vec4 viewSpace = invProjMatrix * nml::vec4(clipSpace, 0.0f, 1.0f);
	nml::vec4 worldSpace = invViewMatrix * viewSpace;

	return nml::vec3(worldSpace) / worldSpace.w;
}

void Renderer::onGridVisibilityToggled(bool showGrid) {
	m_showGrid = showGrid;
}

void Renderer::onBackfaceCullingToggled(bool backfaceCullingEnabled) {
	m_backfaceCullingEnabled = backfaceCullingEnabled;
}

void Renderer::onCamerasVisibilityToggled(bool showCameras) {
	m_showCameras = showCameras;
}

void Renderer::onLightingToggled(bool lightingEnabled) {
	m_lightingEnabled = lightingEnabled;
}

void Renderer::onCollidersVisibilityToggled(bool showColliders) {
	m_showColliders = showColliders;
}

void Renderer::onCameraProjectionSwitched(bool cameraProjectionOrthographic) {
	m_camera.useOrthographicProjection = cameraProjectionOrthographic;
}

void Renderer::onCameraReset() {
	if (!m_camera.useOrthographicProjection) {
		m_camera.perspectivePosition = m_camera.basePerspectivePosition;
		m_camera.perspectiveDirection = m_camera.basePerspectiveDirection;
		m_camera.perspectiveYaw = nml::toDeg(std::atan2(m_camera.perspectiveDirection[2], m_camera.perspectiveDirection[0]));
		m_camera.perspectivePitch = nml::toDeg(-std::asin(m_camera.perspectiveDirection[1]));
	}
	else {
		m_camera.orthographicPosition = m_camera.baseOrthographicPosition;
		m_camera.orthographicDirection = m_camera.baseOrthographicDirection;
		m_camera.orthographicUp = m_camera.baseOrthographicUp;
		m_camera.orthographicHalfExtent = m_camera.baseOrthographicHalfExtent;
	}
}

void Renderer::onOrthographicCameraToAxisChanged(const nml::vec3& axis) {
	m_camera.useOrthographicProjection = true;
	m_camera.orthographicPosition = nml::vec3(0.0f, 0.0f, 0.0f);
	m_camera.orthographicDirection = axis;
	if ((axis.y == -1.0f) || (axis.y == 1.0f)) {
		m_camera.orthographicUp = nml::vec3(0.0f, 0.0f, -1.0f);
	}
	else {
		m_camera.orthographicUp = nml::vec3(0.0f, 1.0f, 0.0f);
	}
}

void Renderer::keyPressEvent(QKeyEvent* event) {
	if (event->isAutoRepeat()) {
		event->accept();
		return;
	}

	if (event->key() == m_cameraForwardKey) {
		m_cameraForwardKeyPressed = true;
	}
	else if (event->key() == m_cameraBackwardKey) {
		m_cameraBackwardKeyPressed = true;
	}
	else if (event->key() == m_cameraLeftKey) {
		m_cameraLeftKeyPressed = true;
	}
	else if (event->key() == m_cameraRightKey) {
		m_cameraRightKeyPressed = true;
	}
	else if (event->key() == m_cameraUpKey) {
		m_cameraUpKeyPressed = true;
	}
	else if (event->key() == m_cameraDownKey) {
		m_cameraDownKeyPressed = true;
	}
	else if (event->key() == m_translateEntityKey) {
		if ((m_globalInfo.currentEntityID != NO_ENTITY) && !m_moveCameraButtonPressed && !anyEntityTransformKeyPressed()) {
			m_translateEntityKeyPressed = true;
			m_entityMoveTransform = m_globalInfo.entities[m_globalInfo.currentEntityID].transform;
			QPoint cursorPos = mapFromGlobal(QCursor::pos());
			m_mouseCursorPreviousPosition = nml::vec2(static_cast<float>(cursorPos.x()), static_cast<float>(height() - cursorPos.y()));
		}
	}
	else if (event->key() == m_rotateEntityKey) {
		if ((m_globalInfo.currentEntityID != NO_ENTITY) && !m_moveCameraButtonPressed && !anyEntityTransformKeyPressed()) {
			m_rotateEntityKeyPressed = true;
			m_entityMoveTransform = m_globalInfo.entities[m_globalInfo.currentEntityID].transform;
			QPoint cursorPos = mapFromGlobal(QCursor::pos());
			m_mouseCursorPreviousPosition = nml::vec2(static_cast<float>(cursorPos.x()), static_cast<float>(height() - cursorPos.y()));
		}
	}
	else if (event->key() == m_scaleEntityKey) {
		if ((m_globalInfo.currentEntityID != NO_ENTITY) && !m_moveCameraButtonPressed && !anyEntityTransformKeyPressed()) {
			m_scaleEntityKeyPressed = true;
			m_entityMoveTransform = m_globalInfo.entities[m_globalInfo.currentEntityID].transform;
			QPoint cursorPos = mapFromGlobal(QCursor::pos());
			m_mouseCursorPreviousPosition = nml::vec2(static_cast<float>(cursorPos.x()), static_cast<float>(height() - cursorPos.y()));
		}
	}
	else if (event->key() == Qt::Key::Key_Delete) {
		if (m_globalInfo.currentEntityID != NO_ENTITY) {
			m_globalInfo.undoStack->push(new DestroyEntityCommand(m_globalInfo, m_globalInfo.currentEntityID));
		}
	}
	event->accept();
}

void Renderer::keyReleaseEvent(QKeyEvent* event) {
	if (event->isAutoRepeat()) {
		event->accept();
		return;
	}

	if (event->key() == m_cameraForwardKey) {
		m_cameraForwardKeyPressed = false;
	}
	else if (event->key() == m_cameraBackwardKey) {
		m_cameraBackwardKeyPressed = false;
	}
	else if (event->key() == m_cameraLeftKey) {
		m_cameraLeftKeyPressed = false;
	}
	else if (event->key() == m_cameraRightKey) {
		m_cameraRightKeyPressed = false;
	}
	else if (event->key() == m_cameraUpKey) {
		m_cameraUpKeyPressed = false;
	}
	else if (event->key() == m_cameraDownKey) {
		m_cameraDownKeyPressed = false;
	}
	else if (event->key() == m_translateEntityKey) {
		if (m_translateEntityKeyPressed) {
			m_translateEntityKeyPressed = false;
			m_mouseCursorDifference = nml::vec2(0.0f, 0.0f);
			if (m_globalInfo.currentEntityID != NO_ENTITY) {
				m_globalInfo.undoStack->push(new ChangeEntityComponentCommand(m_globalInfo, m_globalInfo.currentEntityID, "Transform", &m_entityMoveTransform.value()));
				m_entityMoveTransform.reset();
			}
		}
	}
	else if (event->key() == m_rotateEntityKey) {
		if (m_rotateEntityKeyPressed) {
			m_rotateEntityKeyPressed = false;
			m_mouseCursorDifference = nml::vec2(0.0f, 0.0f);
			if (m_globalInfo.currentEntityID != NO_ENTITY) {
				m_globalInfo.undoStack->push(new ChangeEntityComponentCommand(m_globalInfo, m_globalInfo.currentEntityID, "Transform", &m_entityMoveTransform.value()));
				m_entityMoveTransform.reset();
			}
		}
	}
	else if (event->key() == m_scaleEntityKey) {
		if (m_scaleEntityKeyPressed) {
			m_scaleEntityKeyPressed = false;
			m_mouseCursorDifference = nml::vec2(0.0f, 0.0f);
			if (m_globalInfo.currentEntityID != NO_ENTITY) {
				m_globalInfo.undoStack->push(new ChangeEntityComponentCommand(m_globalInfo, m_globalInfo.currentEntityID, "Transform", &m_entityMoveTransform.value()));
				m_entityMoveTransform.reset();
			}
		}
	}
	event->accept();
}

void Renderer::mousePressEvent(QMouseEvent* event) {
	if (!anyEntityTransformKeyPressed()) {
		if (event->button() == Qt::RightButton) {
			m_moveCameraButtonPressed = true;
			m_savedMousePosition = nml::vec2(static_cast<float>(QCursor::pos().x()), static_cast<float>(QCursor::pos().y()));
			m_mouseCursorPreviousPosition = m_savedMousePosition;
			setCursor(Qt::CursorShape::BlankCursor);
		}
		else if (event->button() == Qt::LeftButton) {
			m_doPicking = true;
		}
	}
	event->accept();
}

void Renderer::mouseReleaseEvent(QMouseEvent* event) {
	if (!anyEntityTransformKeyPressed()) {
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
	event->accept();
}

void Renderer::mouseMoveEvent(QMouseEvent* event) {
	if (!anyEntityTransformKeyPressed()) {
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
			nml::vec2 mouseCursorCurrentPosition = nml::vec2(static_cast<float>(event->pos().x()), static_cast<float>(height()) - static_cast<float>(event->pos().y()));
			if (m_translateEntityKeyPressed) {
				nml::vec3 worldSpaceCursorCurrentPosition = unproject(mouseCursorCurrentPosition, static_cast<float>(width()), static_cast<float>(height()), m_camera.invViewMatrix, m_camera.invProjMatrix);
				nml::vec3 worldSpaceCursorPreviousPosition = unproject(m_mouseCursorPreviousPosition, static_cast<float>(width()), static_cast<float>(height()), m_camera.invViewMatrix, m_camera.invProjMatrix);
				nml::vec3 worldSpaceCursorDifference = worldSpaceCursorCurrentPosition - worldSpaceCursorPreviousPosition;
				if (nml::dot(worldSpaceCursorDifference, worldSpaceCursorDifference) != 0.0f) {
					if (!m_camera.useOrthographicProjection) {
						nml::vec3 cameraEntityDifference = m_entityMoveTransform->position - m_camera.perspectivePosition;
						if (nml::dot(cameraEntityDifference, cameraEntityDifference) != 0.0f) {
							nml::vec3 worldSpaceCursorDifferenceNormalized = nml::normalize(worldSpaceCursorDifference);
							float worldSpaceCursorDifferenceLength = worldSpaceCursorDifference.length();
							float cameraEntityDifferenceLength = cameraEntityDifference.length();
							float coefficient = (cameraEntityDifferenceLength * worldSpaceCursorDifferenceLength) / m_camera.nearPlane;
							m_entityMoveTransform->position += worldSpaceCursorDifferenceNormalized * coefficient;
						}
					}
					else {
						m_entityMoveTransform->position += worldSpaceCursorDifference;
					}
				}
			}
			else if (m_rotateEntityKeyPressed) {
				nml::mat4 rotationMatrix;
				if (!m_camera.useOrthographicProjection) {
					rotationMatrix = nml::rotate((mouseCursorCurrentPosition.x - m_mouseCursorPreviousPosition.x) / static_cast<float>(width()), m_camera.perspectiveDirection);
				}
				else {
					rotationMatrix = nml::rotate((mouseCursorCurrentPosition.x - m_mouseCursorPreviousPosition.x) / static_cast<float>(width()), m_camera.orthographicDirection);
				}
				nml::vec3 rotationAngles = nml::vec3(nml::toDeg(std::atan2(rotationMatrix.z.y, rotationMatrix.z.z)), nml::toDeg(std::atan2(-rotationMatrix.z.x, std::sqrt((rotationMatrix.z.y * rotationMatrix.z.y) + (rotationMatrix.z.z * rotationMatrix.z.z)))), nml::toDeg(std::atan2(rotationMatrix.y.x, rotationMatrix.x.x)));
				m_entityMoveTransform->rotation -= rotationAngles;
				m_entityMoveTransform->rotation = nml::vec3(std::fmod(m_entityMoveTransform->rotation.x, 360.0f), std::fmod(m_entityMoveTransform->rotation.y, 360.0f), std::fmod(m_entityMoveTransform->rotation.z, 360.0f));
			}
			else if (m_scaleEntityKeyPressed) {
				nml::vec3 worldSpaceCursorCurrentPosition = unproject(mouseCursorCurrentPosition, static_cast<float>(width()), static_cast<float>(height()), m_camera.invViewMatrix, m_camera.invProjMatrix);
				nml::vec3 worldSpaceCursorPreviousPosition = unproject(m_mouseCursorPreviousPosition, static_cast<float>(width()), static_cast<float>(height()), m_camera.invViewMatrix, m_camera.invProjMatrix);
				nml::vec3 worldSpaceCursorDifference = worldSpaceCursorCurrentPosition - worldSpaceCursorPreviousPosition;
				if (nml::dot(worldSpaceCursorDifference, worldSpaceCursorDifference) != 0.0f) {
					float worldSpaceCursorDifferenceLength = worldSpaceCursorDifference.length();
					nml::vec3 worldSpaceCursorCurrentEntityDifference = worldSpaceCursorCurrentPosition - m_entityMoveTransform->position;
					if (nml::dot(worldSpaceCursorCurrentEntityDifference, worldSpaceCursorCurrentEntityDifference) != 0.0f) {
						float scaleFactor = 10.0f;
						if (!m_camera.useOrthographicProjection) {
							scaleFactor = 1000.0f;
						}
						m_entityMoveTransform->scale += ((worldSpaceCursorDifferenceLength * scaleFactor) / worldSpaceCursorCurrentEntityDifference.length()) * ((nml::dot(worldSpaceCursorDifference, worldSpaceCursorCurrentEntityDifference) > 0.0) ? 1.0f : -1.0f);
					}
				}
			}
			m_mouseCursorPreviousPosition = mouseCursorCurrentPosition;
		}
	}
	event->accept();
}

void Renderer::wheelEvent(QWheelEvent* event) {
	m_mouseScrollY = static_cast<float>(event->angleDelta().y()) / 120.0f;
	event->accept();
}

void Renderer::focusOutEvent(QFocusEvent* event) {
	m_cameraForwardKeyPressed = false;
	m_cameraBackwardKeyPressed = false;
	m_cameraLeftKeyPressed = false;
	m_cameraRightKeyPressed = false;
	m_cameraUpKeyPressed = false;
	m_cameraDownKeyPressed = false;
	event->accept();
}

void Renderer::resizeEvent(QResizeEvent* event) {
	QOpenGLWidget::resizeEvent(event);
	m_gotResized = true;
}