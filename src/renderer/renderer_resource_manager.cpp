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
#include <algorithm>
#include <fstream>

RendererResourceManager::RendererResourceManager(Localization* passLocalization, Logger* passLogger) : localization(passLocalization), logger(passLogger) {}

void RendererResourceManager::loadModel(const std::string& modelPath, const std::string& name) {
	if (!std::filesystem::exists(modelPath)) {
		logger->addLog(LogLevel::Warning, localization->getString("log_type_file_does_not_exist", { localization->getString("model"), modelPath}));
		return;
	}

	if (modelLastWriteTime.find(modelPath) != modelLastWriteTime.end()) {
		if (modelLastWriteTime[modelPath] == std::filesystem::last_write_time(modelPath)) {
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

	modelLastWriteTime[modelPath] = std::filesystem::last_write_time(modelPath);

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

	if (materialLastWriteTime.find(materialPath) != materialLastWriteTime.end()) {
		if (materialLastWriteTime[materialPath] == std::filesystem::last_write_time(materialPath)) {
			// Resource is already loaded and hasn't changed
			return;
		}
	}

	Material material;
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

	materialLastWriteTime[materialPath] = std::filesystem::last_write_time(materialPath);

	materials[name] = material;
}

void RendererResourceManager::loadImage(const std::string& imagePath, const std::string& name) {
	if (!std::filesystem::exists(imagePath)) {
		logger->addLog(LogLevel::Warning, localization->getString("log_type_file_does_not_exist", { localization->getString("image"), imagePath }));
		return;
	}

	if (imageLastWriteTime.find(imagePath) != imageLastWriteTime.end()) {
		if (imageLastWriteTime[imagePath] == std::filesystem::last_write_time(imagePath)) {
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
		}
		else {
			logger->addLog(LogLevel::Warning, localization->getString("log_type_file_extension_not_supported_editor", { localization->getString("image"), extension }));
		}
	}

	imageLastWriteTime[imagePath] = std::filesystem::last_write_time(imagePath);

	if (!image.data.empty()) {
		imagesToGPU[name] = image;
	}
}

void RendererResourceManager::loadSampler(const std::string& samplerPath, const std::string& name) {
	if (!std::filesystem::exists(samplerPath)) {
		logger->addLog(LogLevel::Warning, localization->getString("log_type_file_does_not_exist", { localization->getString("image_sampler"), samplerPath }));
		return;
	}

	if (samplerLastWriteTime.find(samplerPath) != samplerLastWriteTime.end()) {
		if (samplerLastWriteTime[samplerPath] == std::filesystem::last_write_time(samplerPath)) {
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

	samplerLastWriteTime[samplerPath] = std::filesystem::last_write_time(samplerPath);

	samplersToGPU[name] = sampler;
}

void RendererResourceManager::loadMeshColliders(Mesh& mesh) {
	// Calculate OBB, Sphere and Capsule
	auto uniquePositionsCmp = [](const nml::vec3& a, const nml::vec3& b) {
		return nml::to_string(a) < nml::to_string(b);
		};

	std::set<nml::vec3, decltype(uniquePositionsCmp)> uniquePositions(uniquePositionsCmp);

	for (size_t j = 0; j < mesh.vertices.size(); j++) {
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
	std::sort(eigen.begin(), eigen.end(), [](const std::pair<float, nml::vec3>& a, const std::pair<float, nml::vec3>& b) {
		return a.first > b.first;
		});

	mesh.obb.center = means;
	mesh.sphere.center = means;
	mesh.sphere.radius = 0.0f;
	mesh.capsule.radius = 0.0f;

	float segmentLengthMax = 0.0f;
	for (const nml::vec3& position : uniquePositions) {
		const nml::vec3 positionMinusCenter = position - means;

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
		const float segmentLength = std::abs(nml::dot(eigen[0].second, positionMinusCenter));
		if (segmentLength > segmentLengthMax) {
			segmentLengthMax = segmentLength;
		}

		const float radius = std::abs(nml::dot(eigen[1].second, positionMinusCenter));
		if (radius > mesh.capsule.radius) {
			mesh.capsule.radius = radius;
		}
	}

	// OBB
	nml::mat4 rotationMatrix;
	if ((eigen[0].second == nml::vec3(1.0f, 0.0f, 0.0f)) &&
		(eigen[1].second == nml::vec3(0.0f, 0.0f, 1.0f)) &&
		(eigen[2].second == nml::vec3(0.0f, 1.0f, 0.0f))) {
		rotationMatrix = nml::mat4(nml::vec4(eigen[0].second, 0.0f), nml::vec4(eigen[2].second, 0.0f), nml::vec4(eigen[1].second, 0.0f), nml::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		float tmp = mesh.obb.halfExtent.y;
		mesh.obb.halfExtent.y = mesh.obb.halfExtent.z;
		mesh.obb.halfExtent.z = tmp;
	}
	else if ((eigen[0].second == nml::vec3(0.0f, 1.0f, 0.0f)) &&
		(eigen[1].second == nml::vec3(1.0f, 0.0f, 0.0f)) &&
		(eigen[2].second == nml::vec3(0.0f, 0.0f, 1.0f))) {
		rotationMatrix = nml::mat4(nml::vec4(eigen[1].second, 0.0f), nml::vec4(eigen[0].second, 0.0f), nml::vec4(eigen[2].second, 0.0f), nml::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		float tmp = mesh.obb.halfExtent.x;
		mesh.obb.halfExtent.x = mesh.obb.halfExtent.y;
		mesh.obb.halfExtent.y = tmp;
	}
	else if ((eigen[0].second == nml::vec3(0.0f, 0.0f, 1.0f)) &&
		(eigen[1].second == nml::vec3(1.0f, 0.0f, 0.0f)) &&
		(eigen[2].second == nml::vec3(0.0f, 1.0f, 0.0f))) {
		rotationMatrix = nml::mat4(nml::vec4(eigen[1].second, 0.0f), nml::vec4(eigen[2].second, 0.0f), nml::vec4(eigen[0].second, 0.0f), nml::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		float tmp = mesh.obb.halfExtent.x;
		mesh.obb.halfExtent.x = mesh.obb.halfExtent.y;
		mesh.obb.halfExtent.y = mesh.obb.halfExtent.z;
		mesh.obb.halfExtent.z = tmp;
	}
	else if ((eigen[0].second == nml::vec3(0.0f, 1.0f, 0.0f)) &&
		(eigen[1].second == nml::vec3(0.0f, 0.0f, 1.0f)) &&
		(eigen[2].second == nml::vec3(1.0f, 0.0f, 0.0f))) {
		rotationMatrix = nml::mat4(nml::vec4(eigen[2].second, 0.0f), nml::vec4(eigen[0].second, 0.0f), nml::vec4(eigen[1].second, 0.0f), nml::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		float tmp = mesh.obb.halfExtent.x;
		mesh.obb.halfExtent.x = mesh.obb.halfExtent.z;
		mesh.obb.halfExtent.z = mesh.obb.halfExtent.y;
		mesh.obb.halfExtent.y = tmp;
	}
	else if ((eigen[0].second == nml::vec3(0.0f, 0.0f, 1.0f)) &&
		(eigen[1].second == nml::vec3(0.0f, 1.0f, 0.0f)) &&
		(eigen[2].second == nml::vec3(1.0f, 0.0f, 0.0f))) {
		rotationMatrix = nml::mat4(nml::vec4(eigen[2].second, 0.0f), nml::vec4(eigen[1].second, 0.0f), nml::vec4(eigen[0].second, 0.0f), nml::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		float tmp = mesh.obb.halfExtent.x;
		mesh.obb.halfExtent.x = mesh.obb.halfExtent.z;
		mesh.obb.halfExtent.z = tmp;
	}
	else {
		rotationMatrix = nml::mat4(nml::vec4(eigen[0].second, 0.0f), nml::vec4(eigen[1].second, 0.0f), nml::vec4(eigen[2].second, 0.0f), nml::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	}
	mesh.obb.rotation = nml::rotationMatrixToEulerAngles(rotationMatrix);
	mesh.obb.rotation.x = nml::toDeg(mesh.obb.rotation.x);
	mesh.obb.rotation.y = nml::toDeg(mesh.obb.rotation.y);
	mesh.obb.rotation.z = nml::toDeg(mesh.obb.rotation.z);

	// Sphere
	mesh.sphere.radius = std::sqrt(mesh.sphere.radius);

	// Capsule
	mesh.capsule.base = mesh.obb.center - (eigen[0].second * (segmentLengthMax - mesh.capsule.radius));
	mesh.capsule.tip = mesh.obb.center + (eigen[0].second * (segmentLengthMax - mesh.capsule.radius));

	mesh.collidersCalculated = true;
}

RendererResourceManager::AssetType RendererResourceManager::getFileAssetType(const std::string& path) {
	size_t lastDot = path.rfind('.');
	if (lastDot != std::string::npos) {
		std::string extension = path.substr(lastDot + 1);

		if ((extension == "gltf") ||
			(extension == "glb") ||
			(extension == "obj") ||
			(extension == "ntmd")) {
			return AssetType::Model;
		}
		else if (extension == "ntml") {
			return AssetType::Material;
		}
		else if ((extension == "jpg") ||
			(extension == "jpeg") ||
			(extension == "png") ||
			(extension == "tga") ||
			(extension == "bmp") ||
			(extension == "gif")) {
			return AssetType::Image;
		}
		else if (extension == "ntsp") {
			return AssetType::ImageSampler;
		}
	}

	return AssetType::Unknown;
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
	modelNtmdPrimitiveToMaterialPath[relativeModelPath] = std::vector<std::string>();
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
					modelNtmdPrimitiveToMaterialPath[relativeModelPath].push_back(materialPath);
				}
				else {
					modelNtmdPrimitiveToMaterialPath[relativeModelPath].push_back("");
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
			}

			if (j["vertices"][i].contains("uv")) {
				mesh.vertices[i].uv.x = j["vertices"][i]["uv"][0];
				mesh.vertices[i].uv.y = j["vertices"][i]["uv"][1];
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

	return mesh;
}

void RendererResourceManager::loadNtml(const std::string& materialPath, Material& material) {
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
				loadImage(projectDirectory + "/" + std::string(j["diffuse"]["texture"]["imagePath"]), material.diffuseTextureName);
			}

			if (j["diffuse"]["texture"].contains("imageSamplerPath")) {
				material.diffuseTextureSamplerName = j["diffuse"]["texture"]["imageSamplerPath"];
				loadSampler(projectDirectory + "/" + std::string(j["diffuse"]["texture"]["imageSamplerPath"]), material.diffuseTextureSamplerName);
			}
		}
		else if (j["diffuse"].contains("color")) {
			nml::vec4 diffuseColor = { j["diffuse"]["color"][0], j["diffuse"]["color"][1], j["diffuse"]["color"][2], j["diffuse"]["color"][3] };
			std::string mapKey = "srgb " + std::to_string(diffuseColor.x) + " " + std::to_string(diffuseColor.y) + " " + std::to_string(diffuseColor.z) + " " + std::to_string(diffuseColor.w);

			if (textures.find(mapKey) == textures.end()) {
				ImageToGPU image;
				image.width = 1;
				image.height = 1;
				image.data = { static_cast<uint8_t>(round(255.0f * diffuseColor.x)),
					static_cast<uint8_t>(round(255.0f * diffuseColor.y)),
					static_cast<uint8_t>(round(255.0f * diffuseColor.z)),
					static_cast<uint8_t>(round(255.0f * diffuseColor.w))
				};

				imagesToGPU[mapKey] = image;
			}

			material.diffuseTextureName = mapKey;
		}
	}
	
	if (j.contains("emissive")) {
		if (j["emissive"].contains("texture")) {
			if (j["emissive"]["texture"].contains("imagePath")) {
				material.emissiveTextureName = j["emissive"]["texture"]["imagePath"];
				loadImage(projectDirectory + "/" + std::string(j["emissive"]["texture"]["imagePath"]), material.emissiveTextureName);
			}

			if (j["emissive"]["texture"].contains("imageSamplerPath")) {
				material.emissiveTextureSamplerName = j["emissive"]["texture"]["imageSamplerPath"];
				loadSampler(projectDirectory + "/" + std::string(j["emissive"]["texture"]["imageSamplerPath"]), material.emissiveTextureSamplerName);
			}
		}
		else if (j["emissive"].contains("color")) {
			nml::vec3 emissiveColor = { j["emissive"]["color"][0], j["emissive"]["color"][1], j["emissive"]["color"][2] };
			std::string mapKey = "srgb " + std::to_string(emissiveColor.x) + " " + std::to_string(emissiveColor.y) + " " + std::to_string(emissiveColor.z) + " " + std::to_string(1.0f);

			if (textures.find(mapKey) == textures.end()) {
				ImageToGPU image;
				image.width = 1;
				image.height = 1;
				image.data = { static_cast<uint8_t>(round(255.0f * emissiveColor.x)),
					static_cast<uint8_t>(round(255.0f * emissiveColor.y)),
					static_cast<uint8_t>(round(255.0f * emissiveColor.z)),
					255
				};

				imagesToGPU[mapKey] = image;
			}

			material.emissiveTextureName = mapKey;
		}

		if (j["emissive"].contains("factor")) {
			material.emissiveFactor = j["emissive"]["factor"];
		}
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

	nlohmann::json jsmplr = nlohmann::json::parse(samplerFile);

	if (jsmplr.contains("minFilter")) {
		if (jsmplr["minFilter"] == "Nearest") {
			sampler.minFilter = SamplerToGPU::Filter::Nearest;
		}
		else if (jsmplr["minFilter"] == "Linear") {
			sampler.minFilter = SamplerToGPU::Filter::Linear;
		}

		if (jsmplr["magFilter"] == "Nearest") {
			sampler.magFilter = SamplerToGPU::Filter::Nearest;
		}
		else if (jsmplr["magFilter"] == "Linear") {
			sampler.magFilter = SamplerToGPU::Filter::Linear;
		}

		if (jsmplr["mipmapFilter"] == "Nearest") {
			sampler.mipmapFilter = SamplerToGPU::Filter::Nearest;
		}
		else if (jsmplr["mipmapFilter"] == "Linear") {
			sampler.mipmapFilter = SamplerToGPU::Filter::Linear;
		}

		if (jsmplr["addressModeU"] == "Repeat") {
			sampler.wrapS = SamplerToGPU::Wrap::Repeat;
		}
		else if (jsmplr["addressModeU"] == "MirroredRepeat") {
			sampler.wrapS = SamplerToGPU::Wrap::MirroredRepeat;
		}
		else if (jsmplr["addressModeU"] == "ClampToEdge") {
			sampler.wrapS = SamplerToGPU::Wrap::ClampToEdge;
		}

		if (jsmplr["addressModeV"] == "Repeat") {
			sampler.wrapT = SamplerToGPU::Wrap::Repeat;
		}
		else if (jsmplr["addressModeV"] == "MirroredRepeat") {
			sampler.wrapT = SamplerToGPU::Wrap::MirroredRepeat;
		}
		else if (jsmplr["addressModeV"] == "ClampToEdge") {
			sampler.wrapT = SamplerToGPU::Wrap::ClampToEdge;
		}
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

			size_t positionCount = 0;
			size_t normalCount = 0;
			size_t uvCount = 0;

			size_t positionStride = 0;
			size_t normalStride = 0;
			size_t uvStride = 0;

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
			}
			size_t vertexCount = positionCount;
			primitive.mesh.vertices.resize(vertexCount);

			size_t positionCursor = 0;
			size_t normalCursor = 0;
			size_t uvCursor = 0;

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
									logger->addLog(LogLevel::Warning, localization->getString("log_gltf_invalid_base64_texture", { modelPath, "base color texture"}));
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
							}
							samplersToGPU[sampler.toString()] = sampler;

							primitive.material.diffuseTextureName = imageURI;
							primitive.material.diffuseTextureSamplerName = sampler.toString();
						}
					}
					else if (baseColorFactor != NULL) {
						std::string mapKey = "srgb " + std::to_string(baseColorFactor[0]) + " " + std::to_string(baseColorFactor[1]) + " " + std::to_string(baseColorFactor[2]) + " " + std::to_string(baseColorFactor[3]);

						if (textures.find(mapKey) == textures.end()) {
							ImageToGPU image;
							image.width = 1;
							image.height = 1;
							image.data = { static_cast<uint8_t>(round(255.0f * baseColorFactor[0])),
								static_cast<uint8_t>(round(255.0f * baseColorFactor[1])),
								static_cast<uint8_t>(round(255.0f * baseColorFactor[2])),
								static_cast<uint8_t>(round(255.0f * baseColorFactor[3]))
							};

							imagesToGPU[mapKey] = image;
						}

						primitive.material.diffuseTextureName = mapKey;
					}
				}

				// Emissive texture
				cgltf_texture_view emissiveTextureView = primitiveMaterial->emissive_texture;
				cgltf_texture* emissiveTexture = emissiveTextureView.texture;
				cgltf_float* emissiveFactor = primitiveMaterial->emissive_factor;
				if (emissiveTexture != NULL) {
					cgltf_image* emissiveImage = emissiveTexture->image;
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
								logger->addLog(LogLevel::Warning, localization->getString("log_gltf_invalid_base64_texture", { modelPath, "emissive texture" }));
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
						}
						samplersToGPU[sampler.toString()] = sampler;

						primitive.material.emissiveTextureName = imageURI;
						primitive.material.emissiveTextureSamplerName = sampler.toString();
					}
				}
				else if (emissiveFactor != NULL) {
					std::string mapKey = "srgb " + std::to_string(emissiveFactor[0]) + " " + std::to_string(emissiveFactor[1]) + " " + std::to_string(emissiveFactor[2]) + " " + std::to_string(emissiveFactor[3]);

					if (textures.find(mapKey) == textures.end()) {
						ImageToGPU image;
						image.width = 1;
						image.height = 1;
						image.data = { static_cast<uint8_t>(round(255.0f * emissiveFactor[0])),
							static_cast<uint8_t>(round(255.0f * emissiveFactor[1])),
							static_cast<uint8_t>(round(255.0f * emissiveFactor[2])),
							static_cast<uint8_t>(round(255.0f * emissiveFactor[3]))
						};

						imagesToGPU[mapKey] = image;
					}

					primitive.material.emissiveTextureName = mapKey;
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
	std::unordered_map<std::string, Material> mtlMaterials;

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
				model.primitives.push_back(ModelPrimitive());
				currentPrimitive = &model.primitives.back();
				uniqueVertices.clear();
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
				model.primitives.push_back(ModelPrimitive());
				currentPrimitive = &model.primitives.back();
				uniqueVertices.clear();
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
						}
						// Normal index
						else if (j == 2) {
							vertex.normal = normals[static_cast<size_t>(std::atoi(valueIndices[j].c_str())) - 1];
						}
					}
				}

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

std::unordered_map<std::string, RendererResourceManager::Material> RendererResourceManager::loadMtl(const std::string& materialPath) {
	std::unordered_map<std::string, Material> mtlMaterials;

	std::ifstream file(materialPath);

	// Open file
	if (!file.is_open()) {
		logger->addLog(LogLevel::Warning, localization->getString("log_file_cannot_be_opened", { materialPath }));
		return mtlMaterials;
	}

	Material* currentMaterial = nullptr;

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
			mtlMaterials[tokens[1]] = Material();
			currentMaterial = &mtlMaterials[tokens[1]];
		}
		else if (tokens[0] == "Kd") {
			std::string mapKey = "srgb " + tokens[1] + " " + tokens[2] + " " + tokens[3];

			if (textures.find(mapKey) == textures.end()) {
				ImageToGPU image;
				image.width = 1;
				image.height = 1;
				image.data = { static_cast<uint8_t>(round(255.0f * std::atof(tokens[1].c_str()))),
					static_cast<uint8_t>(round(255.0f * std::atof(tokens[2].c_str()))),
					static_cast<uint8_t>(round(255.0f * std::atof(tokens[3].c_str()))),
					255
				};

				imagesToGPU[mapKey] = image;
			}

			if (currentMaterial) {
				currentMaterial->diffuseTextureName = mapKey;
			}
		}
		else if (tokens[0] == "map_Kd") {
			loadImage(materialDirectory + "/" + tokens[1], materialDirectory + "/" + tokens[1]);
			if (currentMaterial) {
				currentMaterial->diffuseTextureName = materialDirectory + "/" + tokens[1];
			}
		}
		else if (tokens[0] == "Ke") {
			std::string mapKey = "srgb " + tokens[1] + " " + tokens[2] + " " + tokens[3];

			if (textures.find(mapKey) == textures.end()) {
				ImageToGPU image;
				image.width = 1;
				image.height = 1;
				image.data = { static_cast<uint8_t>(round(255.0f * std::atof(tokens[1].c_str()))),
					static_cast<uint8_t>(round(255.0f * std::atof(tokens[2].c_str()))),
					static_cast<uint8_t>(round(255.0f * std::atof(tokens[3].c_str()))),
					255
				};

				imagesToGPU[mapKey] = image;
			}

			if (currentMaterial) {
				currentMaterial->emissiveTextureName = mapKey;
			}
		}
		else if (tokens[0] == "map_Ke") {
			loadImage(materialDirectory + "/" + tokens[1], materialDirectory + "/" + tokens[1]);
			if (currentMaterial) {
				currentMaterial->emissiveTextureName = materialDirectory + "/" + tokens[1];
			}
		}
	}

	file.close();

	return mtlMaterials;
}