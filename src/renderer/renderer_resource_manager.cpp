#include "renderer_resource_manager.h"
#include "../common/asset_helper.h"
#include "../common/localization.h"
#include "../common/logger.h"
#if defined(NTSHENGN_COMPILER_MSVC)
#pragma warning(push)
#pragma warning(disable : 4996)
#endif
#define CGLTF_IMPLEMENTATION
#include "../../external/cgltf/cgltf.h"
#if defined(NTSHENGN_COMPILER_MSVC)
#pragma warning(pop)
#endif
#if defined(NTSHENGN_COMPILER_GCC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-overflow"
#endif
#define STB_IMAGE_IMPLEMENTATION
#include "../../external/stb/stb_image.h"
#if defined(NTSHENGN_COMPILER_GCC)
#pragma GCC diagnostic pop
#endif
#include "../../external/nlohmann/json.hpp"
#include <array>
#include <set>
#include <functional>
#include <numeric>
#include <limits>
#include <algorithm>
#include <fstream>

RendererResourceManager::RendererResourceManager(Localization* passLocalization, Logger* passLogger) : localization(passLocalization), logger(passLogger) {}

void RendererResourceManager::loadModel(const std::string& modelPath, const std::string& name) {
	if (!std::filesystem::exists(modelPath)) {
		logger->addLog(LogLevel::Warning, localization->getString("log_type_file_does_not_exist", { localization->getString("model"), modelPath}));
		return;
	}

	if (modelLastWriteTimes.find(modelPath) != modelLastWriteTimes.end()) {
		if (modelLastWriteTimes[modelPath] == std::filesystem::last_write_time(modelPath)) {
			// Resource is already loaded and hasn't changed
			return;
		}
	}

	Model model;
	size_t lastDot = modelPath.rfind('.');
	if (lastDot != std::string::npos) {
		std::string extension = modelPath.substr(lastDot + 1);
		if ((extension == "gltf") || (extension == "glb")) {
			loadGltf(modelPath, model);
		}
		else if (extension == "obj") {
			loadObj(modelPath, model);
		}
		else if (extension == "ntmd") {
			loadNtmd(modelPath, model);
		}
		else {
			logger->addLog(LogLevel::Warning, localization->getString("log_type_file_extension_not_supported_editor", { localization->getString("model"), extension }));
			return;
		}
	}

	// Calculate AABB
	for (ModelPrimitive& primitive : model.primitives) {
		for (const Mesh::Vertex& vertex : primitive.mesh.vertices) {
			if (vertex.position[0] < primitive.mesh.aabbMin.x) {
				primitive.mesh.aabbMin.x = vertex.position[0];
			}
			if (vertex.position[0] > primitive.mesh.aabbMax.x) {
				primitive.mesh.aabbMax.x = vertex.position[0];
			}

			if (vertex.position[1] < primitive.mesh.aabbMin.y) {
				primitive.mesh.aabbMin.y = vertex.position[1];
			}
			if (vertex.position[1] > primitive.mesh.aabbMax.y) {
				primitive.mesh.aabbMax.y = vertex.position[1];
			}

			if (vertex.position[2] < primitive.mesh.aabbMin.z) {
				primitive.mesh.aabbMin.z = vertex.position[2];
			}
			if (vertex.position[2] > primitive.mesh.aabbMax.z) {
				primitive.mesh.aabbMax.z = vertex.position[2];
			}
		}

		const float epsilon = 0.0001f;

		if (primitive.mesh.aabbMin.x == primitive.mesh.aabbMax.x) {
			primitive.mesh.aabbMin.x -= epsilon;
			primitive.mesh.aabbMax.x += epsilon;
		}

		if (primitive.mesh.aabbMin.y == primitive.mesh.aabbMax.y) {
			primitive.mesh.aabbMin.y -= epsilon;
			primitive.mesh.aabbMax.y += epsilon;
		}

		if (primitive.mesh.aabbMin.z == primitive.mesh.aabbMax.z) {
			primitive.mesh.aabbMin.z -= epsilon;
			primitive.mesh.aabbMax.z += epsilon;
		}
	}

	modelLastWriteTimes[modelPath] = std::filesystem::last_write_time(modelPath);

	if (!model.primitives.empty()) {
		models[name] = model;
		modelsToLoad.push_back(name);
	}
}

void RendererResourceManager::loadMaterial(const std::string& materialPath, const std::string& name) {
	if (!std::filesystem::exists(materialPath)) {
		logger->addLog(LogLevel::Warning, localization->getString("log_type_file_does_not_exist", { localization->getString("material"), materialPath }));
		return;
	}

	if (materialLastWriteTimes.find(materialPath) != materialLastWriteTimes.end()) {
		if (materialLastWriteTimes[materialPath] == std::filesystem::last_write_time(materialPath)) {
			// Resource is already loaded and hasn't changed
			return;
		}
	}

	RendererMaterial material;
	size_t lastDot = materialPath.rfind('.');
	if (lastDot != std::string::npos) {
		std::string extension = materialPath.substr(lastDot + 1);
		if (extension == "ntml") {
			loadNtml(materialPath, material);
		}
		else {
			logger->addLog(LogLevel::Warning, localization->getString("log_type_file_extension_not_supported_editor", { localization->getString("material"), extension }));
			return;
		}
	}

	materialLastWriteTimes[materialPath] = std::filesystem::last_write_time(materialPath);

	materials[name] = material;
}

void RendererResourceManager::loadImage(const std::string& imagePath, const std::string& name) {
	if (!std::filesystem::exists(imagePath)) {
		logger->addLog(LogLevel::Warning, localization->getString("log_type_file_does_not_exist", { localization->getString("image"), imagePath }));
		return;
	}

	ImageToGPU::ColorSpace colorSpace = ImageToGPU::ColorSpace::Linear;
	if (imageLastWriteTimeAndColorSpaces.find(imagePath) != imageLastWriteTimeAndColorSpaces.end()) {
		colorSpace = imageLastWriteTimeAndColorSpaces[imagePath].second;
		if (imageLastWriteTimeAndColorSpaces[imagePath].first == std::filesystem::last_write_time(imagePath)) {
			// Resource is already loaded and hasn't changed
			return;
		}
	}

	ImageToGPU image;
	size_t lastDot = imagePath.rfind('.');
	if (lastDot != std::string::npos) {
		std::string extension = imagePath.substr(lastDot + 1);
		if (extension == "ntim") {
			loadNtim(imagePath, image);
		}
		else if ((extension == "jpg") ||
			(extension == "jpeg") ||
			(extension == "png") ||
			(extension == "tga") ||
			(extension == "bmp") ||
			(extension == "gif")) {
			loadImageStb(imagePath, image);
			image.colorSpace = colorSpace;
		}
		else {
			logger->addLog(LogLevel::Warning, localization->getString("log_type_file_extension_not_supported_editor", { localization->getString("image"), extension }));
		}
	}

	imageLastWriteTimeAndColorSpaces[imagePath].first = std::filesystem::last_write_time(imagePath);
	imageLastWriteTimeAndColorSpaces[imagePath].second = image.colorSpace;

	if (!image.data.empty()) {
		imagesToGPU[name] = image;
	}
}

void RendererResourceManager::loadSampler(const std::string& samplerPath, const std::string& name) {
	if (!std::filesystem::exists(samplerPath)) {
		logger->addLog(LogLevel::Warning, localization->getString("log_type_file_does_not_exist", { localization->getString("image_sampler"), samplerPath }));
		return;
	}

	if (samplerLastWriteTimes.find(samplerPath) != samplerLastWriteTimes.end()) {
		if (samplerLastWriteTimes[samplerPath] == std::filesystem::last_write_time(samplerPath)) {
			// Resource is already loaded and hasn't changed
			return;
		}
	}

	SamplerToGPU sampler;
	size_t lastDot = samplerPath.rfind('.');
	if (lastDot != std::string::npos) {
		std::string extension = samplerPath.substr(lastDot + 1);
		if (extension == "ntsp") {
			loadNtsp(samplerPath, sampler);
		}
		else {
			logger->addLog(LogLevel::Warning, localization->getString("log_type_file_extension_not_supported_editor", { localization->getString("image_sampler"), extension }));
			return;
		}
	}

	samplerLastWriteTimes[samplerPath] = std::filesystem::last_write_time(samplerPath);

	samplersToGPU[name] = sampler;
}

void RendererResourceManager::loadFragmentShader(const std::string& fragmentShaderPath, const std::string& name) {
	if (!std::filesystem::exists(fragmentShaderPath)) {
		logger->addLog(LogLevel::Warning, localization->getString("log_type_file_does_not_exist", { localization->getString("fragment_shader"), fragmentShaderPath }));
		return;
	}

	if (fragmentShaderLastWriteTimes.find(fragmentShaderPath) != fragmentShaderLastWriteTimes.end()) {
		if (fragmentShaderLastWriteTimes[fragmentShaderPath] == std::filesystem::last_write_time(fragmentShaderPath)) {
			// Resource is already loaded and hasn't changed
			return;
		}
	}

	const std::string fragmentShaderPrefix = R"(#version 460

#define SHADOW_MAPPING_CASCADE_COUNT 3

#define NtshEngn_position fragPosition
#define NtshEngn_normal fragTBN[2]
#define NtshEngn_tangent fragTBN[0]
#define NtshEngn_bitangent fragTBN[1]
#define NtshEngn_uv fragUV
#define NtshEngn_color fragColor
#define NtshEngn_tbn fragTBN
#define NtshEngn_diffuseTexture diffuseTextureSampler
#define NtshEngn_normalTexture normalTextureSampler
#define NtshEngn_metalnessTexture metalnessTextureSampler
#define NtshEngn_roughnessTexture roughnessTextureSampler
#define NtshEngn_occlusionTexture occlusionTextureSampler
#define NtshEngn_emissiveTexture emissiveTextureSampler
#define NtshEngn_emissiveFactor emissiveFactor
#define NtshEngn_alphaCutoff alphaCutoff
#define NtshEngn_scaleUV scaleUV
#define NtshEngn_offsetUV offsetUV
#define NtshEngn_useTriplanarMapping useTriplanarMapping
#define NtshEngn_directionalLightCount lights.count.x
#define NtshEngn_directionalLight(i) lights.info[i]
#define NtshEngn_directionalLightShadows(i, p) directionalLightShadows(i, p)
#define NtshEngn_pointLightCount lights.count.y
#define NtshEngn_pointLight(i) lights.info[lights.count.x + i]
#define NtshEngn_pointLightShadows(i, p) pointLightShadows(i, p)
#define NtshEngn_spotLightCount lights.count.z
#define NtshEngn_spotLight(i) lights.info[lights.count.x + lights.count.y + i]
#define NtshEngn_spotLightShadows(i, p) spotLightShadows(i, p)
#define NtshEngn_ambientLightCount lights.count.w
#define NtshEngn_ambientLight(i) lights.info[lights.count.x + lights.count.y + lights.count.z + i]
#define NtshEngn_time time
#define NtshEngn_cameraPosition cameraPosition
#define NtshEngn_width width
#define NtshEngn_height height
#define NtshEngn_useReversedDepth true
#define NtshEngn_outColor outColor
#define NtshEngn_outDepth gl_FragDepth

in vec3 position;
in vec2 uv;
in mat3 tbn;

const mat4 shadowOffset = mat4(
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0
);

struct Light {
	vec3 position;
	vec3 direction;
	vec3 color;
	float intensity;
	vec2 cutoff;
	float distance;
};

struct Shadow {
	mat4 viewProj;
	float splitDepth;
};

in vec3 fragPosition;
in vec2 fragUV;
in vec4 fragColor;
in mat3 fragTBN;

uniform sampler2D diffuseTextureSampler;

uniform sampler2D normalTextureSampler;

uniform sampler2D metalnessTextureSampler;

uniform sampler2D roughnessTextureSampler;

uniform sampler2D occlusionTextureSampler;

uniform sampler2D emissiveTextureSampler;
uniform float emissiveFactor;

uniform float alphaCutoff;

uniform bool useTriplanarMapping;
uniform vec2 scaleUV;
uniform vec2 offsetUV;

uniform vec3 cameraPosition;
uniform mat4 view;

uniform float time;

uniform uint width;
uniform uint height;

uniform sampler2DArray shadowMapSampler;

layout(binding = 0) restrict readonly buffer LightBuffer {
	uvec4 count;
	Light info[];
} lights;

layout(binding = 1) restrict readonly buffer ShadowMapBuffer {
	Shadow info[];
} shadows;

out vec4 outColor;

vec2 sampleCube(vec3 direction, out uint faceIndex) {
	vec3 directionAbs = abs(direction);
	float offset = 0.0f;
	vec2 uv;
	if ((directionAbs.z >= directionAbs.x) && (directionAbs.z >= directionAbs.y)) {
		faceIndex = (direction.z < 0.0) ? 5 : 4;
		offset = 0.5 / directionAbs.z;
		uv = vec2((direction.z < 0.0) ? -direction.x : direction.x, -direction.y);
	}
	else if (directionAbs.y >= directionAbs.x) {
		faceIndex = (direction.y < 0.0) ? 3 : 2;
		offset = 0.5 / directionAbs.y;
		uv = vec2(direction.x, (direction.y < 0.0) ? -direction.z : direction.z);
	}
	else {
		faceIndex = (direction.x < 0.0) ? 1 : 0;
		offset = 0.5 / directionAbs.x;
		uv = vec2((direction.x < 0.0) ? direction.z : -direction.z, -direction.y);
	}

	return (uv * offset) + 0.5;
}

float shadowValue(uint shadowLayer, vec4 shadowCoord, float bias) {
	float shadow = 0.0;
	if ((shadowCoord.z < -1.0) || (shadowCoord.z > 1.0)) {
		return 1.0;
	}

	const vec2 texelSize = 0.75 * (1.0 / vec2(textureSize(shadowMapSampler, 0).xy));
	for (int x = -1; x <= 1; x++) {
		for (int y = -1; y <= 1; y++) {
			const float depth = texture(shadowMapSampler, vec3(shadowCoord.xy + (vec2(x, y) * texelSize), float(shadowLayer))).r;
			if (depth >= (shadowCoord.z - bias)) {
				shadow += 1.0;
			}
		}
	}

	return shadow / 9.0;
}

float shadowCubeValue(uint shadowLayer, uint faceIndex, vec2 uv, float currentDepth, float bias) {
	const float depth = texture(shadowMapSampler, vec3(uv, float(shadowLayer + faceIndex))).r;
	if (depth >= (currentDepth - bias)) {
		return 1.0;
	}

	return 0.0;
}

float directionalLightShadows(uint lightIndex, vec3 position) {
	const uint shadowLayer = lightIndex * SHADOW_MAPPING_CASCADE_COUNT;
	const vec3 viewPosition = vec3(view * vec4(position, 1.0));

	uint cascadeIndex = 0;
	for (uint i = 0; i < SHADOW_MAPPING_CASCADE_COUNT - 1; i++) {
		if (viewPosition.z < shadows.info[shadowLayer + i].splitDepth) {
			cascadeIndex = i + 1;
		}
	}

	const vec4 shadowCoord = (shadowOffset * shadows.info[shadowLayer + cascadeIndex].viewProj) * vec4(position, 1.0);

	return shadowValue(shadowLayer + cascadeIndex, shadowCoord / shadowCoord.w, 0.00005);
}

float pointLightShadows(uint lightIndex, vec3 position) {
	const uint shadowLayer = (lights.count.x * SHADOW_MAPPING_CASCADE_COUNT) + lightIndex;
	const vec3 lightDirection = position - lights.info[lights.count.x + lightIndex].position;

	uint faceIndex;
	const vec2 cubeUV = sampleCube(lightDirection, faceIndex);

	const vec4 shadowCoord = (shadowOffset * shadows.info[shadowLayer + faceIndex].viewProj) * vec4(position, 1.0);

	return shadowCubeValue(shadowLayer, faceIndex, cubeUV, shadowCoord.z / shadowCoord.w, 0.00005);
}

float spotLightShadows(uint lightIndex, vec3 position) {
	const uint shadowLayer = (lights.count.x * SHADOW_MAPPING_CASCADE_COUNT) + (lights.count.y * 6) + lightIndex;
	const vec4 shadowCoord = (shadowOffset * shadows.info[shadowLayer].viewProj) * vec4(position, 1.0);

	return shadowValue(shadowLayer, shadowCoord / shadowCoord.w, 0.00005);
}

)";

	std::string fragmentShaderCode;
	std::ifstream fragmentShaderFile(fragmentShaderPath, std::ios::in | std::ios::ate | std::ios::binary);
	if (!fragmentShaderFile.is_open()) {
		fragmentShaderCode = "";
	}
	fragmentShaderFile.seekg(0);
	fragmentShaderCode = std::string((std::istreambuf_iterator<char>(fragmentShaderFile)), std::istreambuf_iterator<char>());

	fragmentShaderLastWriteTimes[fragmentShaderPath] = std::filesystem::last_write_time(fragmentShaderPath);

	fragmentShadersToGPU[name] = fragmentShaderPrefix + fragmentShaderCode;
}

void RendererResourceManager::loadMeshColliders(Mesh& mesh) {
	// Calculate OBB, Sphere and Capsule
	auto uniquePositionsCmp = [](const nml::vec3& a, const nml::vec3& b) {
		return nml::to_string(a) < nml::to_string(b);
		};

	std::set<nml::vec3, decltype(uniquePositionsCmp)> uniquePositions(uniquePositionsCmp);

	nml::vec3 positionMin = nml::vec3(std::numeric_limits<float>::max());
	nml::vec3 positionMax = nml::vec3(std::numeric_limits<float>::lowest());

	for (size_t j = 0; j < mesh.vertices.size(); j++) {
		if (mesh.vertices[j].position.x < positionMin.x) {
			positionMin.x = mesh.vertices[j].position.x;
		}
		if (mesh.vertices[j].position.x > positionMax.x) {
			positionMax.x = mesh.vertices[j].position.x;
		}
		if (mesh.vertices[j].position.y < positionMin.y) {
			positionMin.y = mesh.vertices[j].position.y;
		}
		if (mesh.vertices[j].position.y > positionMax.y) {
			positionMax.y = mesh.vertices[j].position.y;
		}
		if (mesh.vertices[j].position.z < positionMin.z) {
			positionMin.z = mesh.vertices[j].position.z;
		}
		if (mesh.vertices[j].position.z > positionMax.z) {
			positionMax.z = mesh.vertices[j].position.z;
		}

		uniquePositions.insert(mesh.vertices[j].position);
	}

	float size = static_cast<float>(uniquePositions.size());

	const nml::vec3 means = std::reduce(uniquePositions.begin(), uniquePositions.end(), nml::vec3(0.0f, 0.0f, 0.0f), [](nml::vec3 acc, const nml::vec3& val) { return acc + val; }) / size;

	nml::mat3 covarianceMatrix;
	for (const nml::vec3& position : uniquePositions) {
		covarianceMatrix.x.x += (position.x - means.x) * (position.x - means.x);
		covarianceMatrix.y.y += (position.y - means.y) * (position.y - means.y);
		covarianceMatrix.z.z += (position.z - means.z) * (position.z - means.z);
		covarianceMatrix.x.y += (position.x - means.x) * (position.y - means.y);
		covarianceMatrix.x.z += (position.x - means.x) * (position.z - means.z);
		covarianceMatrix.y.z += (position.y - means.y) * (position.z - means.z);
	}
	covarianceMatrix.x.x /= size;
	covarianceMatrix.y.y /= size;
	covarianceMatrix.z.z /= size;
	covarianceMatrix.x.y /= size;
	covarianceMatrix.x.z /= size;
	covarianceMatrix.y.z /= size;

	covarianceMatrix.y.x = covarianceMatrix.x.y;
	covarianceMatrix.z.x = covarianceMatrix.x.z;
	covarianceMatrix.z.y = covarianceMatrix.y.z;

	std::array<std::pair<float, nml::vec3>, 3> eigen = covarianceMatrix.eigen();

	nml::vec3 center = (positionMin + positionMax) / 2.0f;
	mesh.obb.center = center;
	mesh.sphere.center = center;
	mesh.sphere.radius = 0.0f;
	mesh.capsule.radius = 0.0f;

	std::array<std::pair<float, nml::vec3>, 3> eigenSorted = eigen;
	std::sort(eigenSorted.begin(), eigenSorted.end(), [](const std::pair<float, nml::vec3>& a, const std::pair<float, nml::vec3>& b) {
		return a.first > b.first;
		});

	float segmentLengthMax = 0.0f;
	for (const nml::vec3& position : uniquePositions) {
		const nml::vec3 positionMinusCenter = position - center;

		// OBB
		const float extentX = std::abs(nml::dot(eigen[0].second, positionMinusCenter));
		if (extentX > mesh.obb.halfExtent.x) {
			mesh.obb.halfExtent.x = extentX;
		}

		const float extentY = std::abs(nml::dot(eigen[1].second, positionMinusCenter));
		if (extentY > mesh.obb.halfExtent.y) {
			mesh.obb.halfExtent.y = extentY;
		}

		const float extentZ = std::abs(nml::dot(eigen[2].second, positionMinusCenter));
		if (extentZ > mesh.obb.halfExtent.z) {
			mesh.obb.halfExtent.z = extentZ;
		}

		// Sphere
		const float positionMinusCenterSquaredLength = nml::dot(positionMinusCenter, positionMinusCenter);
		if (positionMinusCenterSquaredLength > mesh.sphere.radius) {
			mesh.sphere.radius = positionMinusCenterSquaredLength;
		}

		// Capsule
		const float segmentLength = std::abs(nml::dot(eigenSorted[0].second, positionMinusCenter));
		if (segmentLength > segmentLengthMax) {
			segmentLengthMax = segmentLength;
		}

		const float radius = std::abs(nml::dot(eigenSorted[1].second, positionMinusCenter));
		if (radius > mesh.capsule.radius) {
			mesh.capsule.radius = radius;
		}
	}

	// OBB
	nml::mat4 rotationMatrix = nml::mat4(nml::vec4(eigen[0].second, 0.0f), nml::vec4(eigen[1].second, 0.0f), nml::vec4(eigen[2].second, 0.0f), nml::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	mesh.obb.rotation = nml::rotationMatrixToEulerAngles(rotationMatrix);
	mesh.obb.rotation.x = nml::toDeg(mesh.obb.rotation.x);
	mesh.obb.rotation.y = nml::toDeg(mesh.obb.rotation.y);
	mesh.obb.rotation.z = nml::toDeg(mesh.obb.rotation.z);

	// Sphere
	mesh.sphere.radius = std::sqrt(mesh.sphere.radius);

	// Capsule
	mesh.capsule.base = center - (eigenSorted[0].second * (segmentLengthMax - mesh.capsule.radius));
	mesh.capsule.tip = center + (eigenSorted[0].second * (segmentLengthMax - mesh.capsule.radius));

	mesh.collidersCalculated = true;
}

void RendererResourceManager::loadNtmd(const std::string& modelPath, Model& model) {
	std::fstream modelFile(modelPath, std::ios::in);
	if (modelFile.is_open()) {
		if (!nlohmann::json::accept(modelFile)) {
			logger->addLog(LogLevel::Warning, localization->getString("log_file_is_not_valid_json", { modelPath }));
			return;
		}
	}
	else {
		logger->addLog(LogLevel::Warning, localization->getString("log_file_cannot_be_opened", { modelPath }));
		return;
	}

	modelFile = std::fstream(modelPath, std::ios::in);

	nlohmann::json j = nlohmann::json::parse(modelFile);

	std::string relativeModelPath = AssetHelper::absoluteToRelative(modelPath, projectDirectory);
	modelNtmdPrimitiveToMaterialPaths[relativeModelPath] = std::vector<std::string>();
	if (j.contains("primitives")) {
		for (size_t i = 0; i < j["primitives"].size(); i++) {
			ModelPrimitive primitive;
			if (j["primitives"][i].contains("meshPath")) {
				primitive.mesh = loadNtmh(projectDirectory + "/" + std::string(j["primitives"][i]["meshPath"]));

				if (j["primitives"][i].contains("materialPath")) {
					std::string materialPath = j["primitives"][i]["materialPath"];
					std::string fullMaterialPath = projectDirectory + "/" + materialPath;
					loadMaterial(fullMaterialPath, materialPath);
					primitive.material = materials[materialPath];
					modelNtmdPrimitiveToMaterialPaths[relativeModelPath].push_back(materialPath);
				}
				else {
					modelNtmdPrimitiveToMaterialPaths[relativeModelPath].push_back("");
				}
			}

			if (!primitive.mesh.vertices.empty()) {
				model.primitives.push_back(primitive);
			}
		}
	}
}

RendererResourceManager::Mesh RendererResourceManager::loadNtmh(const std::string& meshPath) {
	Mesh mesh;

	std::fstream meshFile(meshPath, std::ios::in);
	if (meshFile.is_open()) {
		if (!nlohmann::json::accept(meshFile)) {
			logger->addLog(LogLevel::Warning, localization->getString("log_file_is_not_valid_json", { meshPath }));
			return mesh;
		}
	}
	else {
		logger->addLog(LogLevel::Warning, localization->getString("log_file_cannot_be_opened", { meshPath }));
		return mesh;
	}

	meshFile = std::fstream(meshPath, std::ios::in);

	nlohmann::json j = nlohmann::json::parse(meshFile);

	bool hasNormals = false;
	bool hasUvs = false;
	bool hasTangents = false;

	if (j.contains("vertices")) {
		mesh.vertices.resize(j["vertices"].size());
		for (size_t i = 0; i < j["vertices"].size(); i++) {
			if (j["vertices"][i].contains("position")) {
				mesh.vertices[i].position.x = j["vertices"][i]["position"][0];
				mesh.vertices[i].position.y = j["vertices"][i]["position"][1];
				mesh.vertices[i].position.z = j["vertices"][i]["position"][2];
			}

			if (j["vertices"][i].contains("normal")) {
				mesh.vertices[i].normal.x = j["vertices"][i]["normal"][0];
				mesh.vertices[i].normal.y = j["vertices"][i]["normal"][1];
				mesh.vertices[i].normal.z = j["vertices"][i]["normal"][2];
				hasNormals = true;
			}

			if (j["vertices"][i].contains("uv")) {
				mesh.vertices[i].uv.x = j["vertices"][i]["uv"][0];
				mesh.vertices[i].uv.y = j["vertices"][i]["uv"][1];
				hasUvs = true;
			}

			if (j["vertices"][i].contains("color")) {
				mesh.vertices[i].color.x = j["vertices"][i]["color"][0];
				mesh.vertices[i].color.y = j["vertices"][i]["color"][1];
				mesh.vertices[i].color.z = j["vertices"][i]["color"][2];
				mesh.vertices[i].color.w = j["vertices"][i]["color"][3];
			}

			if (j["vertices"][i].contains("tangent")) {
				mesh.vertices[i].tangent.x = j["vertices"][i]["tangent"][0];
				mesh.vertices[i].tangent.y = j["vertices"][i]["tangent"][1];
				mesh.vertices[i].tangent.z = j["vertices"][i]["tangent"][2];
				mesh.vertices[i].tangent.w = j["vertices"][i]["tangent"][3];
				hasTangents = true;
			}
		}
	}

	if (j.contains("indices")) {
		for (size_t i = 0; i < j["indices"].size(); i++) {
			mesh.indices.push_back(j["indices"][i]);
		}
	}
	else {
		// Calculate indices
		mesh.indices.resize(mesh.vertices.size());
		std::iota(mesh.indices.begin(), mesh.indices.end(), 0);
	}

	// Calculate tangents
	if ((!hasTangents) && (hasNormals && hasUvs)) {
		calculateTangents(mesh);
	}

	return mesh;
}

void RendererResourceManager::loadNtml(const std::string& materialPath, RendererMaterial& material) {
	std::fstream materialFile(materialPath, std::ios::in);
	if (materialFile.is_open()) {
		if (!nlohmann::json::accept(materialFile)) {
			logger->addLog(LogLevel::Warning, localization->getString("log_file_is_not_valid_json", { materialPath }));
			return;
		}
	}
	else {
		logger->addLog(LogLevel::Warning, localization->getString("log_file_cannot_be_opened", { materialPath }));
		return;
	}

	materialFile = std::fstream(materialPath, std::ios::in);

	nlohmann::json j = nlohmann::json::parse(materialFile);

	if (j.contains("diffuse")) {
		if (j["diffuse"].contains("texture")) {
			if (j["diffuse"]["texture"].contains("imagePath")) {
				material.diffuseTextureName = j["diffuse"]["texture"]["imagePath"];
				loadImage(AssetHelper::relativeToAbsolute(material.diffuseTextureName, projectDirectory), material.diffuseTextureName);
				if (imagesToGPU.find(material.diffuseTextureName) != imagesToGPU.end()) {
					imagesToGPU[material.diffuseTextureName].colorSpace = ImageToGPU::ColorSpace::SRGB;
				}
			}

			if (j["diffuse"]["texture"].contains("imageSamplerPath")) {
				material.diffuseTextureSamplerName = j["diffuse"]["texture"]["imageSamplerPath"];
				loadSampler(AssetHelper::relativeToAbsolute(material.diffuseTextureSamplerName, projectDirectory), material.diffuseTextureSamplerName);
			}
		}
		else if (j["diffuse"].contains("color")) {
			nml::vec4 diffuseColor = { j["diffuse"]["color"][0], j["diffuse"]["color"][1], j["diffuse"]["color"][2], j["diffuse"]["color"][3] };
			ImageToGPU image;
			image.width = 1;
			image.height = 1;
			image.colorSpace = ImageToGPU::ColorSpace::SRGB;
			image.data = { static_cast<uint8_t>(round(255.0f * diffuseColor.x)),
				static_cast<uint8_t>(round(255.0f * diffuseColor.y)),
				static_cast<uint8_t>(round(255.0f * diffuseColor.z)),
				static_cast<uint8_t>(round(255.0f * diffuseColor.w))
			};
			std::string imageName = AssetHelper::absoluteToRelative(materialPath, projectDirectory) + "_diffuseTexture";
			imagesToGPU[imageName] = image;
			material.diffuseTextureName = imageName;
		}
	}

	if (j.contains("normal")) {
		if (j["normal"].contains("texture")) {
			if (j["normal"]["texture"].contains("imagePath")) {
				material.normalTextureName = j["normal"]["texture"]["imagePath"];
				loadImage(AssetHelper::relativeToAbsolute(material.normalTextureName, projectDirectory), material.normalTextureName);
				if (imagesToGPU.find(material.normalTextureName) != imagesToGPU.end()) {
					imagesToGPU[material.normalTextureName].colorSpace = ImageToGPU::ColorSpace::Linear;
				}
			}

			if (j["normal"]["texture"].contains("imageSamplerPath")) {
				material.normalTextureSamplerName = j["normal"]["texture"]["imageSamplerPath"];
				loadSampler(AssetHelper::relativeToAbsolute(material.normalTextureSamplerName, projectDirectory), material.normalTextureSamplerName);
			}
		}
	}

	if (j.contains("metalness")) {
		if (j["metalness"].contains("texture")) {
			if (j["metalness"]["texture"].contains("imagePath")) {
				material.metalnessTextureName = j["metalness"]["texture"]["imagePath"];
				loadImage(AssetHelper::relativeToAbsolute(material.metalnessTextureName, projectDirectory), material.metalnessTextureName);
				if (imagesToGPU.find(material.metalnessTextureName) != imagesToGPU.end()) {
					imagesToGPU[material.metalnessTextureName].colorSpace = ImageToGPU::ColorSpace::Linear;
				}
			}

			if (j["metalness"]["texture"].contains("imageSamplerPath")) {
				material.metalnessTextureSamplerName = j["metalness"]["texture"]["imageSamplerPath"];
				loadSampler(AssetHelper::relativeToAbsolute(material.metalnessTextureSamplerName, projectDirectory), material.metalnessTextureSamplerName);
			}
		}
		else if (j["metalness"].contains("value")) {
			float metalnessValue = j["metalness"]["value"];
			ImageToGPU image;
			image.width = 1;
			image.height = 1;
			image.colorSpace = ImageToGPU::ColorSpace::Linear;
			image.data = { static_cast<uint8_t>(round(255.0f * metalnessValue)),
				static_cast<uint8_t>(round(255.0f * metalnessValue)),
				static_cast<uint8_t>(round(255.0f * metalnessValue)),
				static_cast<uint8_t>(round(255.0f * metalnessValue))
			};
			std::string imageName = AssetHelper::absoluteToRelative(materialPath, projectDirectory) + "_metalnessTexture";
			imagesToGPU[imageName] = image;
			material.metalnessTextureName = imageName;
		}
	}

	if (j.contains("roughness")) {
		if (j["roughness"].contains("texture")) {
			if (j["roughness"]["texture"].contains("imagePath")) {
				material.roughnessTextureName = j["roughness"]["texture"]["imagePath"];
				loadImage(AssetHelper::relativeToAbsolute(material.roughnessTextureName, projectDirectory), material.roughnessTextureName);
				if (imagesToGPU.find(material.roughnessTextureName) != imagesToGPU.end()) {
					imagesToGPU[material.roughnessTextureName].colorSpace = ImageToGPU::ColorSpace::Linear;
				}
			}

			if (j["roughness"]["texture"].contains("imageSamplerPath")) {
				material.roughnessTextureSamplerName = j["roughness"]["texture"]["imageSamplerPath"];
				loadSampler(AssetHelper::relativeToAbsolute(material.roughnessTextureSamplerName, projectDirectory), material.roughnessTextureSamplerName);
			}
		}
		else if (j["roughness"].contains("value")) {
			float roughnessValue = j["roughness"]["value"];
			ImageToGPU image;
			image.width = 1;
			image.height = 1;
			image.colorSpace = ImageToGPU::ColorSpace::Linear;
			image.data = { static_cast<uint8_t>(round(255.0f * roughnessValue)),
				static_cast<uint8_t>(round(255.0f * roughnessValue)),
				static_cast<uint8_t>(round(255.0f * roughnessValue)),
				static_cast<uint8_t>(round(255.0f * roughnessValue))
			};
			std::string imageName = AssetHelper::absoluteToRelative(materialPath, projectDirectory) + "_roughnessTexture";
			imagesToGPU[imageName] = image;
			material.roughnessTextureName = imageName;
		}
	}

	if (j.contains("occlusion")) {
		if (j["occlusion"].contains("texture")) {
			if (j["occlusion"]["texture"].contains("imagePath")) {
				material.occlusionTextureName = j["occlusion"]["texture"]["imagePath"];
				loadImage(AssetHelper::relativeToAbsolute(material.occlusionTextureName, projectDirectory), material.occlusionTextureName);
				if (imagesToGPU.find(material.occlusionTextureName) != imagesToGPU.end()) {
					imagesToGPU[material.occlusionTextureName].colorSpace = ImageToGPU::ColorSpace::Linear;
				}
			}

			if (j["occlusion"]["texture"].contains("imageSamplerPath")) {
				material.occlusionTextureSamplerName = j["occlusion"]["texture"]["imageSamplerPath"];
				loadSampler(AssetHelper::relativeToAbsolute(material.occlusionTextureSamplerName, projectDirectory), material.occlusionTextureSamplerName);
			}
		}
		else if (j["occlusion"].contains("value")) {
			float occlusionValue = j["occlusion"]["value"];
			ImageToGPU image;
			image.width = 1;
			image.height = 1;
			image.colorSpace = ImageToGPU::ColorSpace::Linear;
			image.data = { static_cast<uint8_t>(round(255.0f * occlusionValue)),
				static_cast<uint8_t>(round(255.0f * occlusionValue)),
				static_cast<uint8_t>(round(255.0f * occlusionValue)),
				static_cast<uint8_t>(round(255.0f * occlusionValue))
			};
			std::string imageName = AssetHelper::absoluteToRelative(materialPath, projectDirectory) + "_occlusionTexture";
			imagesToGPU[imageName] = image;
			material.occlusionTextureName = imageName;
		}
	}

	if (j.contains("emissive")) {
		if (j["emissive"].contains("texture")) {
			if (j["emissive"]["texture"].contains("imagePath")) {
				material.emissiveTextureName = j["emissive"]["texture"]["imagePath"];
				loadImage(AssetHelper::relativeToAbsolute(material.emissiveTextureName, projectDirectory), material.emissiveTextureName);
				if (imagesToGPU.find(material.emissiveTextureName) != imagesToGPU.end()) {
					imagesToGPU[material.emissiveTextureName].colorSpace = ImageToGPU::ColorSpace::SRGB;
				}
			}

			if (j["emissive"]["texture"].contains("imageSamplerPath")) {
				material.emissiveTextureSamplerName = j["emissive"]["texture"]["imageSamplerPath"];
				loadSampler(AssetHelper::relativeToAbsolute(material.emissiveTextureSamplerName, projectDirectory), material.emissiveTextureSamplerName);
			}
		}
		else if (j["emissive"].contains("color")) {
			nml::vec3 emissiveColor = { j["emissive"]["color"][0], j["emissive"]["color"][1], j["emissive"]["color"][2] };
			ImageToGPU image;
			image.width = 1;
			image.height = 1;
			image.colorSpace = ImageToGPU::ColorSpace::SRGB;
			image.data = { static_cast<uint8_t>(round(255.0f * emissiveColor.x)),
				static_cast<uint8_t>(round(255.0f * emissiveColor.y)),
				static_cast<uint8_t>(round(255.0f * emissiveColor.z)),
				255
			};
			std::string imageName = AssetHelper::absoluteToRelative(materialPath, projectDirectory) + "_emissiveTexture";
			imagesToGPU[imageName] = image;
			material.emissiveTextureName = imageName;
		}

		if (j["emissive"].contains("factor")) {
			material.emissiveFactor = j["emissive"]["factor"];
		}
	}

	if (j.contains("alphaCutoff")) {
		material.alphaCutoff = j["alphaCutoff"];
	}

	if (j.contains("useTriplanarMapping")) {
		material.useTriplanarMapping = j["useTriplanarMapping"];
	}

	if (j.contains("scaleUV")) {
		material.scaleUV.x = j["scaleUV"][0];
		material.scaleUV.y = j["scaleUV"][1];
	}

	if (j.contains("offsetUV")) {
		material.offsetUV.x = j["offsetUV"][0];
		material.offsetUV.y = j["offsetUV"][1];
	}
}

void RendererResourceManager::loadNtim(const std::string& imagePath, ImageToGPU& image) {
	std::fstream imageFile(imagePath, std::ios::in);
	if (imageFile.is_open()) {
		if (!nlohmann::json::accept(imageFile)) {
			logger->addLog(LogLevel::Warning, localization->getString("log_file_cannot_be_opened", { imagePath }));
			return;
		}
	}
	else {
		logger->addLog(LogLevel::Warning, localization->getString("log_file_cannot_be_opened", { imagePath }));
		return;
	}

	imageFile = std::fstream(imagePath, std::ios::in);

	nlohmann::json j = nlohmann::json::parse(imageFile);

	if (j.contains("width")) {
		image.width = static_cast<uint32_t>(j["width"]);
	}

	if (j.contains("height")) {
		image.height = static_cast<uint32_t>(j["height"]);
	}

	if (j.contains("data")) {
		for (size_t i = 0; i < j["data"].size(); i++) {
			image.data.push_back(static_cast<uint8_t>(j["data"][i]));
		}
	}
}

void RendererResourceManager::loadNtsp(const std::string& samplerPath, SamplerToGPU& sampler) {
	std::fstream samplerFile(samplerPath, std::ios::in);
	if (samplerFile.is_open()) {
		if (!nlohmann::json::accept(samplerFile)) {
			logger->addLog(LogLevel::Warning, localization->getString("log_file_cannot_be_opened", { samplerPath }));
			return;
		}
	}
	else {
		logger->addLog(LogLevel::Warning, localization->getString("log_file_cannot_be_opened", { samplerPath }));
		return;
	}

	samplerFile = std::fstream(samplerPath, std::ios::in);

	nlohmann::json j = nlohmann::json::parse(samplerFile);

	if (j.contains("minFilter")) {
		if (j["minFilter"] == "Nearest") {
			sampler.minFilter = SamplerToGPU::Filter::Nearest;
		}
		else if (j["minFilter"] == "Linear") {
			sampler.minFilter = SamplerToGPU::Filter::Linear;
		}
	}

	if (j.contains("magFilter")) {
		if (j["magFilter"] == "Nearest") {
			sampler.magFilter = SamplerToGPU::Filter::Nearest;
		}
		else if (j["magFilter"] == "Linear") {
			sampler.magFilter = SamplerToGPU::Filter::Linear;
		}
	}

	if (j.contains("mipmapFilter")) {
		if (j["mipmapFilter"] == "Nearest") {
			sampler.mipmapFilter = SamplerToGPU::Filter::Nearest;
		}
		else if (j["mipmapFilter"] == "Linear") {
			sampler.mipmapFilter = SamplerToGPU::Filter::Linear;
		}
	}

	if (j.contains("addressModeU")) {
		if (j["addressModeU"] == "Repeat") {
			sampler.wrapS = SamplerToGPU::Wrap::Repeat;
		}
		else if (j["addressModeU"] == "MirroredRepeat") {
			sampler.wrapS = SamplerToGPU::Wrap::MirroredRepeat;
		}
		else if (j["addressModeU"] == "ClampToEdge") {
			sampler.wrapS = SamplerToGPU::Wrap::ClampToEdge;
		}
	}

	if (j.contains("addressModeV")) {
		if (j["addressModeV"] == "Repeat") {
			sampler.wrapT = SamplerToGPU::Wrap::Repeat;
		}
		else if (j["addressModeV"] == "MirroredRepeat") {
			sampler.wrapT = SamplerToGPU::Wrap::MirroredRepeat;
		}
		else if (j["addressModeV"] == "ClampToEdge") {
			sampler.wrapT = SamplerToGPU::Wrap::ClampToEdge;
		}
	}

	if (j.contains("anisotropyLevel")) {
		sampler.anisotropyLevel = j["anisotropyLevel"];
	}
}

void RendererResourceManager::loadGltf(const std::string& modelPath, Model& model) {
	cgltf_options options = {};
	cgltf_data* data = NULL;
	cgltf_result result = cgltf_parse_file(&options, modelPath.c_str(), &data);
	if (result == cgltf_result_success) {
		result = cgltf_load_buffers(&options, data, modelPath.c_str());

		if (result != cgltf_result_success) {
			logger->addLog(LogLevel::Warning, localization->getString("log_gltf_buffers_cannot_be_loaded", { modelPath }));
		}
		else {
			cgltf_scene* scene = data->scene;

			for (size_t i = 0; i < scene->nodes_count; i++) {
				loadGltfNode(modelPath, model, nml::mat4::identity(), scene->nodes[i]);
			}
		}

		cgltf_free(data);
	}
}

void RendererResourceManager::loadGltfNode(const std::string& modelPath, Model& rendererModel, nml::mat4 modelMatrix, cgltf_node* node) {
	nml::mat4 nodeMatrix = nml::mat4::identity();
	if (node->has_matrix) {
		nodeMatrix = nml::mat4(node->matrix);
	}
	else {
		if (node->has_translation) {
			nodeMatrix *= nml::translate(nml::vec3(node->translation));
		}
		if (node->has_rotation) {
			nodeMatrix *= nml::quatToRotationMatrix(nml::quat(node->rotation[3], node->rotation[0], node->rotation[1], node->rotation[2]));
		}
		if (node->has_scale) {
			nodeMatrix *= nml::scale(nml::vec3(node->scale));
		}
	}

	modelMatrix *= nodeMatrix;

	if (node->mesh) {
		cgltf_mesh* nodeMesh = node->mesh;
		for (size_t i = 0; i < nodeMesh->primitives_count; i++) {
			cgltf_primitive nodeMeshPrimitive = nodeMesh->primitives[i];

			ModelPrimitive primitive;

			float* position = nullptr;
			float* normal = nullptr;
			float* uv = nullptr;
			float* color = nullptr;
			float* tangent = nullptr;

			size_t positionCount = 0;
			size_t normalCount = 0;
			size_t uvCount = 0;
			size_t colorCount = 0;
			size_t tangentCount = 0;

			size_t positionStride = 0;
			size_t normalStride = 0;
			size_t uvStride = 0;
			size_t colorStride = 0;
			size_t tangentStride = 0;

			cgltf_type colorType = cgltf_type_vec3;
			cgltf_component_type colorComponentType = cgltf_component_type_r_32f;
			size_t colorComponentSize = sizeof(float);
			uint8_t colorCanalCount = 3;

			for (size_t j = 0; j < nodeMeshPrimitive.attributes_count; j++) {
				cgltf_attribute attribute = nodeMeshPrimitive.attributes[j];
				std::string attributeName = std::string(attribute.name);

				cgltf_accessor* accessor = attribute.data;
				cgltf_buffer_view* bufferView = accessor->buffer_view;
				std::byte* buffer = static_cast<std::byte*>(bufferView->buffer->data);
				std::byte* bufferOffset = buffer + accessor->offset + bufferView->offset;
				if (attributeName == "POSITION") {
					position = reinterpret_cast<float*>(bufferOffset);
					positionCount = attribute.data->count;
					positionStride = std::max(bufferView->stride, 3 * sizeof(float));
				}
				else if (attributeName == "NORMAL") {
					normal = reinterpret_cast<float*>(bufferOffset);
					normalCount = attribute.data->count;
					normalStride = std::max(bufferView->stride, 3 * sizeof(float));
				}
				else if (attributeName == "TEXCOORD_0") {
					uv = reinterpret_cast<float*>(bufferOffset);
					uvCount = attribute.data->count;
					uvStride = std::max(bufferView->stride, 2 * sizeof(float));
				}
				else if (attributeName == "COLOR_0") {
					color = reinterpret_cast<float*>(bufferOffset);
					colorCount = attribute.data->count;
					colorType = accessor->type;
					colorComponentType = accessor->component_type;

					if (colorComponentType == cgltf_component_type_r_32f) {
						colorComponentSize = sizeof(float);
					}
					else if (colorComponentType == cgltf_component_type_r_8u) {
						colorComponentSize = sizeof(uint8_t);
					}
					else if (colorComponentType == cgltf_component_type_r_16u) {
						colorComponentSize = sizeof(uint16_t);
					}
					if (colorType == cgltf_type_vec3) {
						colorCanalCount = 3;
					}
					else if (colorType == cgltf_type_vec4) {
						colorCanalCount = 4;
					}
					colorStride = std::max(bufferView->stride, colorCanalCount * colorComponentSize);
				}
				else if (attributeName == "TANGENT") {
					tangent = reinterpret_cast<float*>(bufferOffset);
					tangentCount = attribute.data->count;
					tangentStride = std::max(bufferView->stride, 4 * sizeof(float));
				}
			}
			size_t vertexCount = positionCount;
			primitive.mesh.vertices.resize(vertexCount);

			size_t positionCursor = 0;
			size_t normalCursor = 0;
			size_t uvCursor = 0;
			size_t colorCursor = 0;
			size_t tangentCursor = 0;

			for (size_t j = 0; j < vertexCount; j++) {
				nml::vec3 vertexPosition = nml::vec3(position + positionCursor);
				primitive.mesh.vertices[j].position = vertexPosition;
				positionCursor += (positionStride / sizeof(float));

				if (normalCount != 0) {
					nml::vec3 vertexNormal = nml::vec3(normal + normalCursor);
					primitive.mesh.vertices[j].normal = vertexNormal;
					normalCursor += (normalStride / sizeof(float));
				}
				else {
					primitive.mesh.vertices[j].normal = nml::vec3(0.0f, 0.0f, 0.0f);
				}

				if (uvCount != 0) {
					primitive.mesh.vertices[j].uv.x = *(uv + uvCursor);
					primitive.mesh.vertices[j].uv.y = *(uv + uvCursor + 1);
					uvCursor += (uvStride / sizeof(float));
				}
				else {
					primitive.mesh.vertices[j].uv = nml::vec2(0.0f, 0.0f);
				}

				if (colorCount != 0) {
					if (colorComponentType == cgltf_component_type_r_32f) {
						primitive.mesh.vertices[j].color.x = *(color + colorCursor);
						primitive.mesh.vertices[j].color.y = *(color + colorCursor + 1);
						primitive.mesh.vertices[j].color.z = *(color + colorCursor + 2);
						if (colorCanalCount == 4) {
							primitive.mesh.vertices[j].color.w = *(color + colorCursor + 3);
						}
						else {
							primitive.mesh.vertices[j].color.w = 0.0f;
						}
					}
					else if (colorComponentType == cgltf_component_type_r_8u) {
						uint8_t* colorPtr = reinterpret_cast<uint8_t*>(color);
						primitive.mesh.vertices[j].color.x = static_cast<float>(*(colorPtr + colorCursor)) / 255.0f;
						primitive.mesh.vertices[j].color.y = static_cast<float>(*(colorPtr + colorCursor + 1)) / 255.0f;
						primitive.mesh.vertices[j].color.z = static_cast<float>(*(colorPtr + colorCursor + 2)) / 255.0f;
						if (colorCanalCount == 4) {
							primitive.mesh.vertices[j].color.w = static_cast<float>(*(colorPtr + colorCursor + 3)) / 255.0f;
						}
						else {
							primitive.mesh.vertices[j].color.w = 0.0f;
						}
					}
					else if (colorComponentType == cgltf_component_type_r_16u) {
						uint16_t* colorPtr = reinterpret_cast<uint16_t*>(color);
						primitive.mesh.vertices[j].color.x = static_cast<float>(*(colorPtr + colorCursor)) / 65535.0f;
						primitive.mesh.vertices[j].color.y = static_cast<float>(*(colorPtr + colorCursor + 1)) / 65535.0f;
						primitive.mesh.vertices[j].color.z = static_cast<float>(*(colorPtr + colorCursor + 2)) / 65535.0f;
						if (colorCanalCount == 4) {
							primitive.mesh.vertices[j].color.w = static_cast<float>(*(colorPtr + colorCursor + 3)) / 65535.0f;
						}
						else {
							primitive.mesh.vertices[j].color.w = 0.0f;
						}
					}
					colorCursor += (colorStride / colorComponentSize);
				}
				else {
					primitive.mesh.vertices[j].color = nml::vec4(0.0f, 0.0f, 0.0f, 0.0f);
				}

				if (tangentCount != 0) {
					primitive.mesh.vertices[j].tangent.x = *(tangent + tangentCursor);
					primitive.mesh.vertices[j].tangent.y = *(tangent + tangentCursor + 1);
					primitive.mesh.vertices[j].tangent.z = *(tangent + tangentCursor + 2);
					primitive.mesh.vertices[j].tangent.w = *(tangent + tangentCursor + 3);
					tangentCursor += (tangentStride / sizeof(float));
				}
				else {
					primitive.mesh.vertices[j].tangent = nml::vec4(0.0f, 0.0f, 0.0f, 0.0f);
				}
			}

			// Indices
			cgltf_accessor* accessor = nodeMeshPrimitive.indices;
			if (accessor != NULL) {
				primitive.mesh.indices.reserve(accessor->count);
				cgltf_buffer_view* bufferView = accessor->buffer_view;
				cgltf_component_type componentType = accessor->component_type;
				std::byte* buffer = static_cast<std::byte*>(bufferView->buffer->data);
				switch (componentType) {
				case cgltf_component_type_r_8:
				{
					int8_t* index = reinterpret_cast<int8_t*>(buffer + accessor->offset + bufferView->offset);
					for (size_t j = 0; j < accessor->count; j++) {
						primitive.mesh.indices.push_back(static_cast<uint32_t>(index[j]));
					}
					break;
				}

				case cgltf_component_type_r_8u:
				{
					uint8_t* index = reinterpret_cast<uint8_t*>(buffer + accessor->offset + bufferView->offset);
					for (size_t j = 0; j < accessor->count; j++) {
						primitive.mesh.indices.push_back(static_cast<uint32_t>(index[j]));
					}
					break;
				}

				case cgltf_component_type_r_16:
				{
					int16_t* index = reinterpret_cast<int16_t*>(buffer + accessor->offset + bufferView->offset);
					for (size_t j = 0; j < accessor->count; j++) {
						primitive.mesh.indices.push_back(static_cast<uint32_t>(index[j]));
					}
					break;
				}

				case cgltf_component_type_r_16u:
				{
					uint16_t* index = reinterpret_cast<uint16_t*>(buffer + accessor->offset + bufferView->offset);
					for (size_t j = 0; j < accessor->count; j++) {
						primitive.mesh.indices.push_back(static_cast<uint32_t>(index[j]));
					}
					break;
				}

				case cgltf_component_type_r_32u:
				{
					uint32_t* index = reinterpret_cast<uint32_t*>(buffer + accessor->offset + bufferView->offset);
					std::copy(index, index + accessor->count, std::back_inserter(primitive.mesh.indices));
					break;
				}

				case cgltf_component_type_r_32f:
				{
					float* index = reinterpret_cast<float*>(buffer + accessor->offset + bufferView->offset);
					for (size_t j = 0; j < accessor->count; j++) {
						primitive.mesh.indices.push_back(static_cast<uint32_t>(index[j]));
					}
					break;
				}

				default:
					logger->addLog(LogLevel::Warning, localization->getString("log_gltf_index_component_type_invalid", { modelPath }));
				}
			}
			else {
				// Calculate indices
				primitive.mesh.indices.resize(primitive.mesh.vertices.size());
				std::iota(primitive.mesh.indices.begin(), primitive.mesh.indices.end(), 0);
			}

			// Tangents
			if ((tangentCount == 0) && (uvCount != 0) && (normalCount != 0) && (primitive.mesh.indices.size() != 0)) {
				calculateTangents(primitive.mesh);

				// Invert tangent handedness
				for (Mesh::Vertex& vertex : primitive.mesh.vertices) {
					vertex.tangent.w *= -1.0f;
				}
			}

			// Material
			cgltf_material* primitiveMaterial = nodeMeshPrimitive.material;
			if (primitiveMaterial != NULL) {
				if (primitiveMaterial->has_pbr_metallic_roughness) {
					cgltf_pbr_metallic_roughness pbrMetallicRoughness = primitiveMaterial->pbr_metallic_roughness;

					// Base Color / Diffuse texture
					cgltf_texture_view baseColorTextureView = pbrMetallicRoughness.base_color_texture;
					cgltf_texture* baseColorTexture = baseColorTextureView.texture;
					cgltf_float* baseColorFactor = pbrMetallicRoughness.base_color_factor;
					if (baseColorTexture != NULL) {
						cgltf_image* baseColorImage = baseColorTexture->image;
						bool hasImage = false;
						if (baseColorImage->uri) {
							std::string imageURI = baseColorImage->uri;

							size_t base64Pos = imageURI.find(";base64,");
							if (base64Pos != std::string::npos) {
								cgltf_options options = {};

								const std::string uriBase64 = imageURI.substr(base64Pos + 8);
								const size_t decodedDataSize = ((3 * uriBase64.size()) / 4) - std::count(uriBase64.begin(), uriBase64.end(), '=');
								std::vector<uint8_t> decodedData(decodedDataSize);
								cgltf_result result = cgltf_load_buffer_base64(&options, decodedDataSize, uriBase64.c_str(), reinterpret_cast<void**>(decodedData.data()));
								if (result == cgltf_result_success) {
									loadImageFromMemory(decodedData.data(), decodedDataSize, imageURI);
								}
								else {
									logger->addLog(LogLevel::Warning, localization->getString("log_gltf_invalid_base64_texture", { modelPath, localization->getString("log_gltf_base_color_texture", {}) }));
								}
							}
							else {
								std::string modelDirectory = modelPath;
								size_t lastSlashPos = modelDirectory.rfind("/");
								if (lastSlashPos != std::string::npos) {
									modelDirectory = modelDirectory.substr(0, lastSlashPos);
								}
								loadImage(modelDirectory + "/" + imageURI, modelDirectory + "/" + imageURI);
								imageURI = modelDirectory + "/" + imageURI;
							}

							primitive.material.diffuseTextureName = imageURI;

							hasImage = true;
						}
						else if (baseColorImage->buffer_view) {
							cgltf_buffer_view* bufferView = baseColorImage->buffer_view;
							std::byte* buffer = static_cast<std::byte*>(bufferView->buffer->data) + bufferView->offset;
							std::string imageName = modelPath + ":" + std::to_string(rendererModel.primitives.size()) + ":diffuse";
							if (bufferView->name) {
								imageName = modelPath + ":" + std::to_string(rendererModel.primitives.size()) + std::string(bufferView->name) + ":diffuse";
							}
							loadImageFromMemory(buffer, bufferView->size, imageName);

							primitive.material.diffuseTextureName = imageName;

							hasImage = true;
						}

						if (hasImage) {
							if (imagesToGPU.find(primitive.material.diffuseTextureName) != imagesToGPU.end()) {
								imagesToGPU[primitive.material.diffuseTextureName].colorSpace = ImageToGPU::ColorSpace::SRGB;
							}

							SamplerToGPU sampler;
							if (baseColorTexture->sampler != NULL) {
								if ((baseColorTexture->sampler->min_filter == 9728) || (baseColorTexture->sampler->min_filter == 9984) || (baseColorTexture->sampler->min_filter == 9986)) {
									sampler.minFilter = SamplerToGPU::Filter::Nearest;
								}
								else {
									sampler.minFilter = SamplerToGPU::Filter::Linear;
								}

								if ((baseColorTexture->sampler->mag_filter == 9728) || (baseColorTexture->sampler->mag_filter == 9984) || (baseColorTexture->sampler->mag_filter == 9986)) {
									sampler.magFilter = SamplerToGPU::Filter::Nearest;
								}
								else {
									sampler.magFilter = SamplerToGPU::Filter::Linear;
								}

								if ((baseColorTexture->sampler->min_filter == 9984) || (baseColorTexture->sampler->min_filter == 9985)) {
									sampler.mipmapFilter = SamplerToGPU::Filter::Nearest;
								}
								else {
									sampler.mipmapFilter = SamplerToGPU::Filter::Linear;
								}

								if (baseColorTexture->sampler->wrap_s == 10497) {
									sampler.wrapS = SamplerToGPU::Wrap::Repeat;
								}
								else if (baseColorTexture->sampler->wrap_s == 33648) {
									sampler.wrapS = SamplerToGPU::Wrap::MirroredRepeat;
								}

								if (baseColorTexture->sampler->wrap_t == 10497) {
									sampler.wrapT = SamplerToGPU::Wrap::Repeat;
								}
								else if (baseColorTexture->sampler->wrap_t == 33648) {
									sampler.wrapT = SamplerToGPU::Wrap::MirroredRepeat;
								}

								sampler.anisotropyLevel = 16.0f;
							}
							samplersToGPU[sampler.toString()] = sampler;

							primitive.material.diffuseTextureSamplerName = sampler.toString();
						}
					}
					else if (baseColorFactor != NULL) {
						nml::vec4 diffuseColor = nml::vec4(baseColorFactor);
						ImageToGPU image;
						image.width = 1;
						image.height = 1;
						image.colorSpace = ImageToGPU::ColorSpace::SRGB;
						image.data = { static_cast<uint8_t>(round(255.0f * diffuseColor.x)),
							static_cast<uint8_t>(round(255.0f * diffuseColor.y)),
							static_cast<uint8_t>(round(255.0f * diffuseColor.z)),
							static_cast<uint8_t>(round(255.0f * diffuseColor.w))
						};
						std::string imageName = modelPath + ":" + std::to_string(rendererModel.primitives.size()) + ":diffuse";
						imagesToGPU[imageName] = image;
						primitive.material.diffuseTextureName = imageName;
					}

					// Metallic Roughness texture
					cgltf_texture_view metallicRoughnessTextureView = pbrMetallicRoughness.metallic_roughness_texture;
					cgltf_texture* metallicRoughnessTexture = metallicRoughnessTextureView.texture;
					cgltf_float metallicFactor = pbrMetallicRoughness.metallic_factor;
					cgltf_float roughnessFactor = pbrMetallicRoughness.roughness_factor;
					if (metallicRoughnessTexture != NULL) {
						cgltf_image* metallicRoughnessImage = metallicRoughnessTexture->image;
						bool hasImage = false;
						if (metallicRoughnessImage->uri) {
							std::string imageURI = metallicRoughnessImage->uri;

							size_t base64Pos = imageURI.find(";base64,");
							if (base64Pos != std::string::npos) {
								cgltf_options options = {};

								const std::string uriBase64 = imageURI.substr(base64Pos + 8);
								const size_t decodedDataSize = ((3 * uriBase64.size()) / 4) - std::count(uriBase64.begin(), uriBase64.end(), '=');
								std::vector<uint8_t> decodedData(decodedDataSize);
								cgltf_result result = cgltf_load_buffer_base64(&options, decodedDataSize, uriBase64.c_str(), reinterpret_cast<void**>(decodedData.data()));
								if (result == cgltf_result_success) {
									loadImageFromMemory(decodedData.data(), decodedDataSize, imageURI);
								}
								else {
									logger->addLog(LogLevel::Warning, localization->getString("log_gltf_invalid_base64_texture", { modelPath, localization->getString("log_gltf_base_color_texture", {}) }));
								}
							}
							else {
								std::string modelDirectory = modelPath;
								size_t lastSlashPos = modelDirectory.rfind("/");
								if (lastSlashPos != std::string::npos) {
									modelDirectory = modelDirectory.substr(0, lastSlashPos);
								}
								loadImage(modelDirectory + "/" + imageURI, modelDirectory + "/" + imageURI);
								imageURI = modelDirectory + "/" + imageURI;
							}

							primitive.material.metalnessTextureName = imageURI;

							primitive.material.roughnessTextureName = imageURI;

							hasImage = true;
						}
						else if (metallicRoughnessImage->buffer_view) {
							cgltf_buffer_view* bufferView = metallicRoughnessImage->buffer_view;
							std::byte* buffer = static_cast<std::byte*>(bufferView->buffer->data) + bufferView->offset;
							std::string imageName = modelPath + ":" + std::to_string(rendererModel.primitives.size()) + ":metallicRoughness";
							if (bufferView->name) {
								imageName = modelPath + ":" + std::to_string(rendererModel.primitives.size()) + std::string(bufferView->name) + ":metallicRoughness";
							}
							loadImageFromMemory(buffer, bufferView->size, imageName);

							primitive.material.metalnessTextureName = imageName;

							primitive.material.roughnessTextureName = imageName;

							hasImage = true;
						}

						if (hasImage) {
							if (imagesToGPU.find(primitive.material.metalnessTextureName) != imagesToGPU.end()) {
								imagesToGPU[primitive.material.metalnessTextureName].colorSpace = ImageToGPU::ColorSpace::Linear;
							}

							SamplerToGPU sampler;
							if (metallicRoughnessTexture->sampler != NULL) {
								if ((metallicRoughnessTexture->sampler->min_filter == 9728) || (metallicRoughnessTexture->sampler->min_filter == 9984) || (metallicRoughnessTexture->sampler->min_filter == 9986)) {
									sampler.minFilter = SamplerToGPU::Filter::Nearest;
								}
								else {
									sampler.minFilter = SamplerToGPU::Filter::Linear;
								}

								if ((metallicRoughnessTexture->sampler->mag_filter == 9728) || (metallicRoughnessTexture->sampler->mag_filter == 9984) || (metallicRoughnessTexture->sampler->mag_filter == 9986)) {
									sampler.magFilter = SamplerToGPU::Filter::Nearest;
								}
								else {
									sampler.magFilter = SamplerToGPU::Filter::Linear;
								}

								if ((metallicRoughnessTexture->sampler->min_filter == 9984) || (metallicRoughnessTexture->sampler->min_filter == 9985)) {
									sampler.mipmapFilter = SamplerToGPU::Filter::Nearest;
								}
								else {
									sampler.mipmapFilter = SamplerToGPU::Filter::Linear;
								}

								if (metallicRoughnessTexture->sampler->wrap_s == 10497) {
									sampler.wrapS = SamplerToGPU::Wrap::Repeat;
								}
								else if (metallicRoughnessTexture->sampler->wrap_s == 33648) {
									sampler.wrapS = SamplerToGPU::Wrap::MirroredRepeat;
								}

								if (metallicRoughnessTexture->sampler->wrap_t == 10497) {
									sampler.wrapT = SamplerToGPU::Wrap::Repeat;
								}
								else if (metallicRoughnessTexture->sampler->wrap_t == 33648) {
									sampler.wrapT = SamplerToGPU::Wrap::MirroredRepeat;
								}

								sampler.anisotropyLevel = 16.0f;
							}
							samplersToGPU[sampler.toString()] = sampler;

							primitive.material.metalnessTextureSamplerName = sampler.toString();
							primitive.material.roughnessTextureSamplerName = sampler.toString();
						}
					}
					else {
						ImageToGPU image;
						image.width = 1;
						image.height = 1;
						image.colorSpace = ImageToGPU::ColorSpace::Linear;
						image.data = { 0,
							static_cast<uint8_t>(round(255.0f * roughnessFactor)),
							static_cast<uint8_t>(round(255.0f * metallicFactor)),
							0
						};
						std::string imageName = modelPath + ":" + std::to_string(rendererModel.primitives.size()) + ":metallicRoughness";
						imagesToGPU[imageName] = image;
						primitive.material.roughnessTextureName = imageName;
						primitive.material.metalnessTextureName = imageName;
					}
				}

				// Normal texture
				cgltf_texture_view normalTextureView = primitiveMaterial->normal_texture;
				cgltf_texture* normalTexture = normalTextureView.texture;
				if (normalTexture != NULL) {
					cgltf_image* normalImage = normalTexture->image;
					bool hasImage = false;
					if (normalImage->uri) {
						std::string imageURI = normalImage->uri;

						size_t base64Pos = imageURI.find(";base64,");
						if (base64Pos != std::string::npos) {
							cgltf_options options = {};

							const std::string uriBase64 = imageURI.substr(base64Pos + 8);
							const size_t decodedDataSize = ((3 * uriBase64.size()) / 4) - std::count(uriBase64.begin(), uriBase64.end(), '=');
							std::vector<uint8_t> decodedData(decodedDataSize);
							cgltf_result result = cgltf_load_buffer_base64(&options, decodedDataSize, uriBase64.c_str(), reinterpret_cast<void**>(decodedData.data()));
							if (result == cgltf_result_success) {
								loadImageFromMemory(decodedData.data(), decodedDataSize, imageURI);
							}
							else {
								logger->addLog(LogLevel::Warning, localization->getString("log_gltf_invalid_base64_texture", { modelPath, localization->getString("log_gltf_base_color_texture", {}) }));
							}
						}
						else {
							std::string modelDirectory = modelPath;
							size_t lastSlashPos = modelDirectory.rfind("/");
							if (lastSlashPos != std::string::npos) {
								modelDirectory = modelDirectory.substr(0, lastSlashPos);
							}
							loadImage(modelDirectory + "/" + imageURI, modelDirectory + "/" + imageURI);
							imageURI = modelDirectory + "/" + imageURI;
						}

						primitive.material.normalTextureName = imageURI;

						hasImage = true;
					}
					else if (normalImage->buffer_view) {
						cgltf_buffer_view* bufferView = normalImage->buffer_view;
						std::byte* buffer = static_cast<std::byte*>(bufferView->buffer->data) + bufferView->offset;
						std::string imageName = modelPath + ":" + std::to_string(rendererModel.primitives.size()) + ":normal";
						if (bufferView->name) {
							imageName = modelPath + ":" + std::to_string(rendererModel.primitives.size()) + std::string(bufferView->name) + ":normal";
						}
						loadImageFromMemory(buffer, bufferView->size, imageName);

						primitive.material.normalTextureName = imageName;

						hasImage = true;
					}

					if (hasImage) {
						if (imagesToGPU.find(primitive.material.normalTextureName) != imagesToGPU.end()) {
							imagesToGPU[primitive.material.normalTextureName].colorSpace = ImageToGPU::ColorSpace::Linear;
						}

						SamplerToGPU sampler;
						if (normalTexture->sampler != NULL) {
							if ((normalTexture->sampler->min_filter == 9728) || (normalTexture->sampler->min_filter == 9984) || (normalTexture->sampler->min_filter == 9986)) {
								sampler.minFilter = SamplerToGPU::Filter::Nearest;
							}
							else {
								sampler.minFilter = SamplerToGPU::Filter::Linear;
							}

							if ((normalTexture->sampler->mag_filter == 9728) || (normalTexture->sampler->mag_filter == 9984) || (normalTexture->sampler->mag_filter == 9986)) {
								sampler.magFilter = SamplerToGPU::Filter::Nearest;
							}
							else {
								sampler.magFilter = SamplerToGPU::Filter::Linear;
							}

							if ((normalTexture->sampler->min_filter == 9984) || (normalTexture->sampler->min_filter == 9985)) {
								sampler.mipmapFilter = SamplerToGPU::Filter::Nearest;
							}
							else {
								sampler.mipmapFilter = SamplerToGPU::Filter::Linear;
							}

							if (normalTexture->sampler->wrap_s == 10497) {
								sampler.wrapS = SamplerToGPU::Wrap::Repeat;
							}
							else if (normalTexture->sampler->wrap_s == 33648) {
								sampler.wrapS = SamplerToGPU::Wrap::MirroredRepeat;
							}

							if (normalTexture->sampler->wrap_t == 10497) {
								sampler.wrapT = SamplerToGPU::Wrap::Repeat;
							}
							else if (normalTexture->sampler->wrap_t == 33648) {
								sampler.wrapT = SamplerToGPU::Wrap::MirroredRepeat;
							}

							sampler.anisotropyLevel = 16.0f;
						}
						samplersToGPU[sampler.toString()] = sampler;

						primitive.material.normalTextureSamplerName = sampler.toString();
					}
				}

				// Emissive texture
				cgltf_texture_view emissiveTextureView = primitiveMaterial->emissive_texture;
				cgltf_texture* emissiveTexture = emissiveTextureView.texture;
				cgltf_float* emissiveFactor = primitiveMaterial->emissive_factor;
				if (emissiveTexture != NULL) {
					cgltf_image* emissiveImage = emissiveTexture->image;
					bool hasImage = false;
					if (emissiveImage->uri) {
						std::string imageURI = emissiveImage->uri;

						size_t base64Pos = imageURI.find(";base64,");
						if (base64Pos != std::string::npos) {
							cgltf_options options = {};

							const std::string uriBase64 = imageURI.substr(base64Pos + 8);
							const size_t decodedDataSize = ((3 * uriBase64.size()) / 4) - std::count(uriBase64.begin(), uriBase64.end(), '=');
							std::vector<uint8_t> decodedData(decodedDataSize);
							cgltf_result result = cgltf_load_buffer_base64(&options, decodedDataSize, uriBase64.c_str(), reinterpret_cast<void**>(decodedData.data()));
							if (result == cgltf_result_success) {
								loadImageFromMemory(decodedData.data(), decodedDataSize, imageURI);
							}
							else {
								logger->addLog(LogLevel::Warning, localization->getString("log_gltf_invalid_base64_texture", { modelPath, localization->getString("log_gltf_emissive_texture", {}) }));
							}
						}
						else {
							std::string modelDirectory = modelPath;
							size_t lastSlashPos = modelDirectory.rfind("/");
							if (lastSlashPos != std::string::npos) {
								modelDirectory = modelDirectory.substr(0, lastSlashPos);
							}
							loadImage(modelDirectory + "/" + imageURI, modelDirectory + "/" + imageURI);
							imageURI = modelDirectory + "/" + imageURI;
						}

						hasImage = true;

						primitive.material.emissiveTextureName = imageURI;
					}
					else if (emissiveImage->buffer_view) {
						cgltf_buffer_view* bufferView = emissiveImage->buffer_view;
						std::byte* buffer = static_cast<std::byte*>(bufferView->buffer->data) + bufferView->offset;
						std::string imageName = modelPath + ":" + std::to_string(rendererModel.primitives.size()) + ":emissive";
						if (bufferView->name) {
							imageName = modelPath + ":" + std::to_string(rendererModel.primitives.size()) + std::string(bufferView->name) + ":emissive";
						}
						loadImageFromMemory(buffer, bufferView->size, imageName);

						primitive.material.emissiveTextureName = imageName;

						hasImage = true;
					}

					if (hasImage) {
						if (imagesToGPU.find(primitive.material.emissiveTextureName) != imagesToGPU.end()) {
							imagesToGPU[primitive.material.emissiveTextureName].colorSpace = ImageToGPU::ColorSpace::SRGB;
						}

						SamplerToGPU sampler;
						if (emissiveTexture->sampler != NULL) {
							if ((emissiveTexture->sampler->min_filter == 9728) || (emissiveTexture->sampler->min_filter == 9984) || (emissiveTexture->sampler->min_filter == 9986)) {
								sampler.minFilter = SamplerToGPU::Filter::Nearest;
							}
							else {
								sampler.minFilter = SamplerToGPU::Filter::Linear;
							}

							if ((emissiveTexture->sampler->mag_filter == 9728) || (emissiveTexture->sampler->mag_filter == 9984) || (emissiveTexture->sampler->mag_filter == 9986)) {
								sampler.magFilter = SamplerToGPU::Filter::Nearest;
							}
							else {
								sampler.magFilter = SamplerToGPU::Filter::Linear;
							}

							if ((emissiveTexture->sampler->min_filter == 9984) || (emissiveTexture->sampler->min_filter == 9985)) {
								sampler.mipmapFilter = SamplerToGPU::Filter::Nearest;
							}
							else {
								sampler.mipmapFilter = SamplerToGPU::Filter::Linear;
							}

							if (emissiveTexture->sampler->wrap_s == 10497) {
								sampler.wrapS = SamplerToGPU::Wrap::Repeat;
							}
							else if (emissiveTexture->sampler->wrap_s == 33648) {
								sampler.wrapS = SamplerToGPU::Wrap::MirroredRepeat;
							}

							if (emissiveTexture->sampler->wrap_t == 10497) {
								sampler.wrapT = SamplerToGPU::Wrap::Repeat;
							}
							else if (emissiveTexture->sampler->wrap_t == 33648) {
								sampler.wrapT = SamplerToGPU::Wrap::MirroredRepeat;
							}

							sampler.anisotropyLevel = 16.0f;
						}
						samplersToGPU[sampler.toString()] = sampler;

						primitive.material.emissiveTextureSamplerName = sampler.toString();
					}
				}
				else if (emissiveFactor != NULL) {
					 nml::vec3 emissiveColor = nml::vec3(emissiveFactor);
					 ImageToGPU image;
					 image.width = 1;
					 image.height = 1;
					 image.colorSpace = ImageToGPU::ColorSpace::SRGB;
					 image.data = { static_cast<uint8_t>(round(255.0f * emissiveColor.x)),
						 static_cast<uint8_t>(round(255.0f * emissiveColor.y)),
						 static_cast<uint8_t>(round(255.0f * emissiveColor.z)),
						 static_cast<uint8_t>(0.0f)
					 };
					 std::string imageName = modelPath + ":" + std::to_string(rendererModel.primitives.size()) + ":emissive";
					 imagesToGPU[imageName] = image;
					 primitive.material.emissiveTextureName = imageName;
				}

				// Occlusion texture
				cgltf_texture_view occlusionTextureView = primitiveMaterial->occlusion_texture;
				cgltf_texture* occlusionTexture = occlusionTextureView.texture;
				if (occlusionTexture != NULL) {
					cgltf_image* occlusionImage = occlusionTexture->image;
					bool hasImage = false;
					if (occlusionImage->uri) {
						std::string imageURI = occlusionImage->uri;

						size_t base64Pos = imageURI.find(";base64,");
						if (base64Pos != std::string::npos) {
							cgltf_options options = {};

							const std::string uriBase64 = imageURI.substr(base64Pos + 8);
							const size_t decodedDataSize = ((3 * uriBase64.size()) / 4) - std::count(uriBase64.begin(), uriBase64.end(), '=');
							std::vector<uint8_t> decodedData(decodedDataSize);
							cgltf_result result = cgltf_load_buffer_base64(&options, decodedDataSize, uriBase64.c_str(), reinterpret_cast<void**>(decodedData.data()));
							if (result == cgltf_result_success) {
								loadImageFromMemory(decodedData.data(), decodedDataSize, imageURI);
							}
							else {
								logger->addLog(LogLevel::Warning, localization->getString("log_gltf_invalid_base64_texture", { modelPath, localization->getString("log_gltf_emissive_texture", {}) }));
							}
						}
						else {
							std::string modelDirectory = modelPath;
							size_t lastSlashPos = modelDirectory.rfind("/");
							if (lastSlashPos != std::string::npos) {
								modelDirectory = modelDirectory.substr(0, lastSlashPos);
							}
							loadImage(modelDirectory + "/" + imageURI, modelDirectory + "/" + imageURI);
							imageURI = modelDirectory + "/" + imageURI;
						}

						hasImage = true;

						primitive.material.occlusionTextureName = imageURI;
					}
					else if (occlusionImage->buffer_view) {
						cgltf_buffer_view* bufferView = occlusionImage->buffer_view;
						std::byte* buffer = static_cast<std::byte*>(bufferView->buffer->data) + bufferView->offset;
						std::string imageName = modelPath + ":" + std::to_string(rendererModel.primitives.size()) + ":occlusion";
						if (bufferView->name) {
							imageName = modelPath + ":" + std::to_string(rendererModel.primitives.size()) + std::string(bufferView->name) + ":occlusion";
						}
						loadImageFromMemory(buffer, bufferView->size, imageName);

						primitive.material.occlusionTextureName = imageName;

						hasImage = true;
					}

					if (hasImage) {
						if (imagesToGPU.find(primitive.material.occlusionTextureName) != imagesToGPU.end()) {
							imagesToGPU[primitive.material.occlusionTextureName].colorSpace = ImageToGPU::ColorSpace::Linear;
						}

						SamplerToGPU sampler;
						if (occlusionTexture->sampler != NULL) {
							if ((occlusionTexture->sampler->min_filter == 9728) || (occlusionTexture->sampler->min_filter == 9984) || (occlusionTexture->sampler->min_filter == 9986)) {
								sampler.minFilter = SamplerToGPU::Filter::Nearest;
							}
							else {
								sampler.minFilter = SamplerToGPU::Filter::Linear;
							}

							if ((occlusionTexture->sampler->mag_filter == 9728) || (occlusionTexture->sampler->mag_filter == 9984) || (occlusionTexture->sampler->mag_filter == 9986)) {
								sampler.magFilter = SamplerToGPU::Filter::Nearest;
							}
							else {
								sampler.magFilter = SamplerToGPU::Filter::Linear;
							}

							if ((occlusionTexture->sampler->min_filter == 9984) || (occlusionTexture->sampler->min_filter == 9985)) {
								sampler.mipmapFilter = SamplerToGPU::Filter::Nearest;
							}
							else {
								sampler.mipmapFilter = SamplerToGPU::Filter::Linear;
							}

							if (occlusionTexture->sampler->wrap_s == 10497) {
								sampler.wrapS = SamplerToGPU::Wrap::Repeat;
							}
							else if (occlusionTexture->sampler->wrap_s == 33648) {
								sampler.wrapS = SamplerToGPU::Wrap::MirroredRepeat;
							}

							if (occlusionTexture->sampler->wrap_t == 10497) {
								sampler.wrapT = SamplerToGPU::Wrap::Repeat;
							}
							else if (occlusionTexture->sampler->wrap_t == 33648) {
								sampler.wrapT = SamplerToGPU::Wrap::MirroredRepeat;
							}

							sampler.anisotropyLevel = 16.0f;
						}
						samplersToGPU[sampler.toString()] = sampler;

						primitive.material.occlusionTextureSamplerName = sampler.toString();
					}
				}

				// Emissive factor
				if (primitiveMaterial->has_emissive_strength) {
					primitive.material.emissiveFactor = primitiveMaterial->emissive_strength.emissive_strength;
				}

				// Alpha cutoff
				if (primitiveMaterial->alpha_mode == cgltf_alpha_mode_mask) {
					primitive.material.alphaCutoff = primitiveMaterial->alpha_cutoff;
				}
			}

			if (node->name) {
				primitive.name = node->name;
			}

			primitive.modelMatrix = modelMatrix;

			rendererModel.primitives.push_back(primitive);
		}
	}

	for (size_t i = 0; i < node->children_count; i++) {
		loadGltfNode(modelPath, rendererModel, modelMatrix, node->children[i]);
	}
}

void RendererResourceManager::loadImageStb(const std::string& imagePath, ImageToGPU& image) {
	int width;
	int height;
	int texChannels;

	stbi_uc* pixels = stbi_load(imagePath.c_str(), &width, &height, &texChannels, STBI_rgb_alpha);
	if (!pixels) {
		logger->addLog(LogLevel::Warning, localization->getString("log_cannot_load_image", { imagePath }));
		return;
	}

	image.width = static_cast<uint32_t>(width);
	image.height = static_cast<uint32_t>(height);
	image.data.resize(width * height * 4);
	std::copy(pixels, pixels + (width * height * 4), image.data.begin());

	stbi_image_free(pixels);
}

void RendererResourceManager::loadImageFromMemory(void* data, size_t size, const std::string& name) {
	int width;
	int height;
	int texChannels;

	stbi_uc* pixels = stbi_load_from_memory(reinterpret_cast<stbi_uc*>(data), static_cast<int>(size), &width, &height, &texChannels, STBI_rgb_alpha);
	if (!pixels) {
		logger->addLog(LogLevel::Warning, localization->getString("log_cannot_load_image_from_memory"));
		return;
	}

	ImageToGPU image;
	image.width = static_cast<uint32_t>(width);
	image.height = static_cast<uint32_t>(height);
	image.data.resize(width * height * 4);
	std::copy(pixels, pixels + (width * height * 4), image.data.begin());
	imagesToGPU[name] = image;

	stbi_image_free(pixels);
}

void RendererResourceManager::loadObj(const std::string& modelPath, Model& model) {
	std::ifstream file(modelPath);

	// Open file
	if (!file.is_open()) {
		logger->addLog(LogLevel::Warning, localization->getString("log_file_cannot_be_opened", { modelPath }));
		return;
	}

	std::vector<nml::vec3> positions;
	std::vector<nml::vec3> normals;
	std::vector<nml::vec2> uvs;

	std::unordered_map<std::string, uint32_t> uniqueVertices;

	model.primitives.push_back(ModelPrimitive());
	ModelPrimitive* currentPrimitive = &model.primitives.back();

	std::string modelDirectory = modelPath.substr(0, modelPath.rfind('/'));
	std::unordered_map<std::string, RendererMaterial> mtlMaterials;

	bool hasNormals = false;
	bool hasUvs = false;

	std::string line;
	while (std::getline(file, line)) {
		// Ignore comment
		if (line[0] == '#') {
			continue;
		}

		// Parse line
		std::vector<std::string> tokens;
		size_t spacePosition = 0;
		while ((spacePosition = line.find(' ')) != std::string::npos) {
			tokens.push_back(line.substr(0, spacePosition));
			line.erase(0, spacePosition + 1);
		}
		tokens.push_back(line);

		// Parse tokens
		// Position
		if (tokens[0] == "v") {
			positions.push_back({
				static_cast<float>(std::atof(tokens[1].c_str())),
				static_cast<float>(std::atof(tokens[2].c_str())),
				static_cast<float>(std::atof(tokens[3].c_str()))
				});
		}
		// Normal
		else if (tokens[0] == "vn") {
			normals.push_back({
				static_cast<float>(std::atof(tokens[1].c_str())),
				static_cast<float>(std::atof(tokens[2].c_str())),
				static_cast<float>(std::atof(tokens[3].c_str()))
				});
		}
		// UV
		else if (tokens[0] == "vt") {
			uvs.push_back({
				static_cast<float>(std::atof(tokens[1].c_str())),
				static_cast<float>(std::atof(tokens[2].c_str()))
				});
		}
		// Object
		else if (tokens[0] == "o") {
			if (!currentPrimitive->mesh.indices.empty()) {
				if (hasNormals && hasUvs) {
					calculateTangents(currentPrimitive->mesh);
				}
				model.primitives.push_back(ModelPrimitive());
				currentPrimitive = &model.primitives.back();
				uniqueVertices.clear();
				hasNormals = false;
				hasUvs = false;
			}
			if (tokens.size() > 1) {
				currentPrimitive->name = tokens[1];
			}
		}
		// Material
		else if (tokens[0] == "mtllib") {
			std::string materialFile = modelDirectory + "/" + tokens[1];
			mtlMaterials = loadMtl(materialFile);
		}
		else if (tokens[0] == "usemtl") {
			if (!currentPrimitive->mesh.indices.empty()) {
				if (hasNormals && hasUvs) {
					calculateTangents(currentPrimitive->mesh);
				}
				model.primitives.push_back(ModelPrimitive());
				currentPrimitive = &model.primitives.back();
				uniqueVertices.clear();
				hasNormals = false;
				hasUvs = false;
			}
			if ((tokens.size() > 1) && currentPrimitive->name.empty()) {
				currentPrimitive->name = tokens[1];
			}
			if (mtlMaterials.find(tokens[1]) != mtlMaterials.end()) {
				currentPrimitive->material = mtlMaterials[tokens[1]];
			}
		}
		// Face
		else if (tokens[0] == "f") {
			std::vector<uint32_t> tmpIndices;
			for (size_t i = 1; i < tokens.size(); i++) {
				Mesh::Vertex vertex = {};

				std::string tmp = tokens[i];
				std::vector<std::string> valueIndices;
				size_t slashPosition = 0;
				while ((slashPosition = tmp.find('/')) != std::string::npos) {
					valueIndices.push_back(tmp.substr(0, slashPosition));
					tmp.erase(0, slashPosition + 1);
				}
				valueIndices.push_back(tmp);

				for (size_t j = 0; j < valueIndices.size(); j++) {
					if (!valueIndices[j].empty()) {
						// v/vt/vn
						// Position index
						if (j == 0) {
							vertex.position = positions[static_cast<size_t>(std::atoi(valueIndices[j].c_str())) - 1];
						}
						// UV index
						else if (j == 1) {
							vertex.uv = uvs[static_cast<size_t>(std::atoi(valueIndices[j].c_str())) - 1];
							hasUvs = true;
						}
						// Normal index
						else if (j == 2) {
							vertex.normal = normals[static_cast<size_t>(std::atoi(valueIndices[j].c_str())) - 1];
							hasNormals = true;
						}
					}
				}

				if (!hasUvs) {
					vertex.uv = nml::vec2(0.5f, 0.5f);
				}
				vertex.tangent = nml::vec4(0.5f, 0.5f, 0.5f, 1.0f);

				if (uniqueVertices.count(tokens[i]) == 0) {
					uniqueVertices[tokens[i]] = static_cast<uint32_t>(currentPrimitive->mesh.vertices.size());
					currentPrimitive->mesh.vertices.push_back(vertex);
				}
				tmpIndices.push_back(uniqueVertices[tokens[i]]);
			}

			// Face can be a triangle, a rectangle or a N-gons
			// Triangle
			if (tmpIndices.size() == 3) {
				currentPrimitive->mesh.indices.insert(currentPrimitive->mesh.indices.end(), std::make_move_iterator(tmpIndices.begin()), std::make_move_iterator(tmpIndices.end()));
			}
			// Rectangle
			else if (tmpIndices.size() == 4) {
				// Triangle 1
				currentPrimitive->mesh.indices.push_back(tmpIndices[0]);
				currentPrimitive->mesh.indices.push_back(tmpIndices[1]);
				currentPrimitive->mesh.indices.push_back(tmpIndices[2]);

				// Triangle 2
				currentPrimitive->mesh.indices.push_back(tmpIndices[0]);
				currentPrimitive->mesh.indices.push_back(tmpIndices[2]);
				currentPrimitive->mesh.indices.push_back(tmpIndices[3]);
			}
			// N-gons
			else if (tmpIndices.size() > 4) {
				for (size_t i = 2; i < tmpIndices.size(); i++) {
					currentPrimitive->mesh.indices.push_back(tmpIndices[0]);
					currentPrimitive->mesh.indices.push_back(tmpIndices[i - 1]);
					currentPrimitive->mesh.indices.push_back(tmpIndices[i]);
				}
			}
		}
	}

	file.close();
}

std::unordered_map<std::string, RendererMaterial> RendererResourceManager::loadMtl(const std::string& materialPath) {
	std::unordered_map<std::string, RendererMaterial> mtlMaterials;

	std::ifstream file(materialPath);

	// Open file
	if (!file.is_open()) {
		logger->addLog(LogLevel::Warning, localization->getString("log_file_cannot_be_opened", { materialPath }));
		return mtlMaterials;
	}

	RendererMaterial* currentMaterial = nullptr;

	std::string materialDirectory = materialPath.substr(0, materialPath.rfind('/'));

	std::string line;
	while (std::getline(file, line)) {
		// Ignore comment
		if (line[0] == '#') {
			continue;
		}

		// Parse line
		std::vector<std::string> tokens;
		size_t spacePosition = 0;
		while ((spacePosition = line.find(' ')) != std::string::npos) {
			tokens.push_back(line.substr(0, spacePosition));
			line.erase(0, spacePosition + 1);
		}
		tokens.push_back(line);

		// Parse tokens
		if (tokens[0] == "newmtl") {
			mtlMaterials[tokens[1]] = RendererMaterial();
			currentMaterial = &mtlMaterials[tokens[1]];
		}
		else if (tokens[0] == "Kd") {
			if (currentMaterial) {
				ImageToGPU image;
				image.width = 1;
				image.height = 1;
				image.colorSpace = ImageToGPU::ColorSpace::SRGB;
				image.data = { static_cast<uint8_t>(round(255.0f * std::atof(tokens[1].c_str()))),
					static_cast<uint8_t>(round(255.0f * std::atof(tokens[2].c_str()))),
					static_cast<uint8_t>(round(255.0f * std::atof(tokens[3].c_str()))),
					static_cast<uint8_t>(round(255.0f))
				};
				std::string imageName = AssetHelper::absoluteToRelative(materialPath, projectDirectory) + "_diffuseTexture";
				imagesToGPU[imageName] = image;
				currentMaterial->diffuseTextureName = imageName;
			}
		}
		else if (tokens[0] == "map_Kd") {
			loadImage(materialDirectory + "/" + tokens[1], materialDirectory + "/" + tokens[1]);
			if (currentMaterial) {
				currentMaterial->diffuseTextureName = materialDirectory + "/" + tokens[1];
				if (imagesToGPU.find(currentMaterial->diffuseTextureName) != imagesToGPU.end()) {
					imagesToGPU[currentMaterial->diffuseTextureName].colorSpace = ImageToGPU::ColorSpace::SRGB;
				}
			}
		}
		else if (tokens[0] == "Ke") {
			if (currentMaterial) {
				ImageToGPU image;
				image.width = 1;
				image.height = 1;
				image.colorSpace = ImageToGPU::ColorSpace::SRGB;
				image.data = { static_cast<uint8_t>(round(255.0f * std::atof(tokens[1].c_str()))),
					static_cast<uint8_t>(round(255.0f * std::atof(tokens[2].c_str()))),
					static_cast<uint8_t>(round(255.0f * std::atof(tokens[3].c_str()))),
					static_cast<uint8_t>(round(255.0f))
				};
				std::string imageName = AssetHelper::absoluteToRelative(materialPath, projectDirectory) + "_emissiveTexture";
				imagesToGPU[imageName] = image;
				currentMaterial->emissiveTextureName = imageName;
			}
		}
		else if (tokens[0] == "map_Ke") {
			loadImage(materialDirectory + "/" + tokens[1], materialDirectory + "/" + tokens[1]);
			if (currentMaterial) {
				currentMaterial->emissiveTextureName = materialDirectory + "/" + tokens[1];
				if (imagesToGPU.find(currentMaterial->emissiveTextureName) != imagesToGPU.end()) {
					imagesToGPU[currentMaterial->emissiveTextureName].colorSpace = ImageToGPU::ColorSpace::SRGB;
				}
			}
		}
	}

	file.close();

	return mtlMaterials;
}

void RendererResourceManager::calculateTangents(Mesh& mesh) {
	std::vector<nml::vec3> tan1(mesh.vertices.size());
	std::vector<nml::vec3> tan2(mesh.vertices.size());
	for (size_t i = 0; i < mesh.indices.size(); i += 3) {
		const Mesh::Vertex& vertex0 = mesh.vertices[mesh.indices[i]];
		const Mesh::Vertex& vertex1 = mesh.vertices[mesh.indices[i + 1]];
		const Mesh::Vertex& vertex2 = mesh.vertices[mesh.indices[i + 2]];

		const nml::vec3 dPos1 = vertex1.position - vertex0.position;
		const nml::vec3 dPos2 = vertex2.position - vertex0.position;

		const nml::vec2 dUV1 = vertex1.uv - vertex0.uv;
		const nml::vec2 dUV2 = vertex2.uv - vertex0.uv;

		const float r = 1.0f / (dUV1.x * dUV2.y - dUV1.y * dUV2.x);

		const nml::vec3 uDir = (dPos1 * dUV2.y - dPos2 * dUV1.y) * r;
		const nml::vec3 vDir = (dPos2 * dUV1.x - dPos1 * dUV2.x) * r;

		tan1[mesh.indices[i]] += uDir;
		tan1[mesh.indices[i + 1]] += uDir;
		tan1[mesh.indices[i + 2]] += uDir;

		tan2[mesh.indices[i]] += vDir;
		tan2[mesh.indices[i + 1]] += vDir;
		tan2[mesh.indices[i + 2]] += vDir;
	}

	for (size_t i = 0; i < mesh.vertices.size(); i++) {
		const nml::vec3 n = mesh.vertices[i].normal;
		const nml::vec3 t = tan1[i];

		const nml::vec3 tangent = nml::normalize(t - n * nml::dot(n, t));
		const float tangentHandedness = (nml::dot(nml::cross(n, t), tan2[i]) < 0.0f) ? -1.0f : 1.0f;

		mesh.vertices[i].tangent = { tangent.x, tangent.y, tangent.z, tangentHandedness };
	}
}