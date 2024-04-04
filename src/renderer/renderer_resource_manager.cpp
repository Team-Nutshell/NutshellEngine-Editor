#include "renderer_resource_manager.h"
#pragma warning(push)
#pragma warning(disable : 4996)
#define CGLTF_IMPLEMENTATION
#include "../../external/cgltf/cgltf.h"
#pragma warning(pop)
#define STB_IMAGE_IMPLEMENTATION
#include "../../external/stb/stb_image.h"
#include "../../external/nlohmann/json.hpp"
#include <numeric>
#include <algorithm>
#include <fstream>

void RendererResourceManager::loadModel(const std::string& modelPath, const std::string& name) {
	size_t lastDot = modelPath.rfind('.');
	if (lastDot != std::string::npos) {
		std::string extension = modelPath.substr(lastDot + 1);
		if ((extension == "gltf") || (extension == "glb")) {
			loadGltf(modelPath, name);
		}
		else if (extension == "ntmd") {
			loadNtmd(modelPath, name);
		}
		else {
			std::cout << "Model file extension \"." << extension << "\" is not supported." << std::endl;
		}
	}
}

void RendererResourceManager::loadImage(const std::string& imagePath, const std::string& name) {
	size_t lastDot = imagePath.rfind('.');
	if (lastDot != std::string::npos) {
		std::string extension = imagePath.substr(lastDot + 1);
		if (extension == "ntim") {
			loadNtim(imagePath, name);
		}
		else {
			loadImageStb(imagePath, name);
		}
	}
}

void RendererResourceManager::loadNtmd(const std::string& modelPath, const std::string& name) {
	ModelToGPU model;

	std::fstream modelFile(modelPath, std::ios::in);
	if (modelFile.is_open()) {
		if (!nlohmann::json::accept(modelFile)) {
			std::cout << "\"" << modelPath << "\" is not a valid JSON file." << std::endl;
			return;
		}
	}
	else {
		std::cout << "\"" << modelPath << "\" cannot be opened." << std::endl;
		return;
	}

	modelFile = std::fstream(modelPath, std::ios::in);

	nlohmann::json j = nlohmann::json::parse(modelFile);

	if (j.contains("primitives")) {
		for (size_t i = 0; i < j["primitives"].size(); i++) {
			if (j["primitives"][i].contains("meshPath")) {
				MeshToGPU mesh = loadNtmh(projectDirectory + "/" + std::string(j["primitives"][i]["meshPath"]));
				if (!mesh.vertices.empty()) {
					model.meshes.push_back(mesh);
				}

				if (j["primitives"][i].contains("materialPath")) {
					std::string materialPath = projectDirectory + "/" + std::string(j["primitives"][i]["materialPath"]);
					std::fstream materialFile(materialPath, std::ios::in);
					if (materialFile.is_open()) {
						if (!nlohmann::json::accept(materialFile)) {
							std::cout << "\"" << materialPath << "\" is not a valid JSON file." << std::endl;
							return;
						}
					}
					else {
						std::cout << "\"" << materialPath << "\" cannot be opened." << std::endl;
						return;
					}

					materialFile = std::fstream(materialPath, std::ios::in);

					nlohmann::json jmtl = nlohmann::json::parse(materialFile);

					if (jmtl.contains("diffuseTexture")) {
						if (jmtl["diffuseTexture"].contains("imagePath")) {
							model.meshes.back().diffuseTexturePath = jmtl["diffuseTexture"]["imagePath"];
							loadImage(projectDirectory + "/" + std::string(jmtl["diffuseTexture"]["imagePath"]), model.meshes.back().diffuseTexturePath);
						}

						if (jmtl["diffuseTexture"].contains("imageSamplerPath")) {
							std::string samplerPath = projectDirectory + "/" + std::string(jmtl["diffuseTexture"]["imageSamplerPath"]);
							std::fstream samplerFile(samplerPath, std::ios::in);
							if (samplerFile.is_open()) {
								if (!nlohmann::json::accept(samplerFile)) {
									std::cout << "\"" << samplerPath << "\" is not a valid JSON file." << std::endl;
									return;
								}
							}
							else {
								std::cout << "\"" << samplerPath << "\" cannot be opened." << std::endl;
								return;
							}

							samplerFile = std::fstream(samplerPath, std::ios::in);

							nlohmann::json jsmplr = nlohmann::json::parse(samplerFile);

							ImageToGPU& image = imagesToGPU[jmtl["diffuseTexture"]["imagePath"]];
							if (jsmplr.contains("minFilter")) {
								if (jsmplr["minFilter"] == "Nearest") {
									image.minFilter = ImageToGPU::SamplerFilter::Nearest;
								}
								else if (jsmplr["minFilter"] == "Linear") {
									image.minFilter = ImageToGPU::SamplerFilter::Linear;
								}

								if (jsmplr["magFilter"] == "Nearest") {
									image.magFilter = ImageToGPU::SamplerFilter::Nearest;
								}
								else if (jsmplr["magFilter"] == "Linear") {
									image.magFilter = ImageToGPU::SamplerFilter::Linear;
								}

								if (jsmplr["addressModeU"] == "Repeat") {
									image.wrapS = ImageToGPU::SamplerWrap::Repeat;
								}
								else if (jsmplr["addressModeU"] == "MirroredRepeat") {
									image.wrapS = ImageToGPU::SamplerWrap::MirroredRepeat;
								}
								else if (jsmplr["addressModeU"] == "ClampToEdge") {
									image.wrapS = ImageToGPU::SamplerWrap::ClampToEdge;
								}

								if (jsmplr["addressModeV"] == "Repeat") {
									image.wrapT = ImageToGPU::SamplerWrap::Repeat;
								}
								else if (jsmplr["addressModeV"] == "MirroredRepeat") {
									image.wrapT = ImageToGPU::SamplerWrap::MirroredRepeat;
								}
								else if (jsmplr["addressModeV"] == "ClampToEdge") {
									image.wrapT = ImageToGPU::SamplerWrap::ClampToEdge;
								}
							}
						}
					}
					
					if (jmtl.contains("emissiveTexture")) {
						if (jmtl["emissiveTexture"].contains("imagePath")) {
							model.meshes.back().emissiveTexturePath = jmtl["emissiveTexture"]["imagePath"];
							loadImage(projectDirectory + "/" + std::string(jmtl["emissiveTexture"]["imagePath"]), model.meshes.back().emissiveTexturePath);
						}

						if (jmtl["emissiveTexture"].contains("imageSamplerPath")) {
							std::string samplerPath = projectDirectory + "/" + std::string(jmtl["emissiveTexture"]["imageSamplerPath"]);
							std::fstream samplerFile(samplerPath, std::ios::in);
							if (samplerFile.is_open()) {
								if (!nlohmann::json::accept(samplerFile)) {
									std::cout << "\"" << samplerPath << "\" is not a valid JSON file." << std::endl;
									return;
								}
							}
							else {
								std::cout << "\"" << samplerPath << "\" cannot be opened." << std::endl;
								return;
							}

							samplerFile = std::fstream(samplerPath, std::ios::in);

							nlohmann::json jsmplr = nlohmann::json::parse(samplerFile);

							ImageToGPU& image = imagesToGPU[jmtl["emissiveTexture"]["imagePath"]];
							if (jsmplr.contains("minFilter")) {
								if (jsmplr["minFilter"] == "Nearest") {
									image.minFilter = ImageToGPU::SamplerFilter::Nearest;
								}
								else if (jsmplr["minFilter"] == "Linear") {
									image.minFilter = ImageToGPU::SamplerFilter::Linear;
								}

								if (jsmplr["magFilter"] == "Nearest") {
									image.magFilter = ImageToGPU::SamplerFilter::Nearest;
								}
								else if (jsmplr["magFilter"] == "Linear") {
									image.magFilter = ImageToGPU::SamplerFilter::Linear;
								}

								if (jsmplr["addressModeU"] == "Repeat") {
									image.wrapS = ImageToGPU::SamplerWrap::Repeat;
								}
								else if (jsmplr["addressModeU"] == "MirroredRepeat") {
									image.wrapS = ImageToGPU::SamplerWrap::MirroredRepeat;
								}
								else if (jsmplr["addressModeU"] == "ClampToEdge") {
									image.wrapS = ImageToGPU::SamplerWrap::ClampToEdge;
								}

								if (jsmplr["addressModeV"] == "Repeat") {
									image.wrapT = ImageToGPU::SamplerWrap::Repeat;
								}
								else if (jsmplr["addressModeV"] == "MirroredRepeat") {
									image.wrapT = ImageToGPU::SamplerWrap::MirroredRepeat;
								}
								else if (jsmplr["addressModeV"] == "ClampToEdge") {
									image.wrapT = ImageToGPU::SamplerWrap::ClampToEdge;
								}
							}
						}
					}
				}
			}
		}
	}

	if (!model.meshes.empty()) {
		modelsToGPU[name] = model;
	}
}

RendererResourceManager::MeshToGPU RendererResourceManager::loadNtmh(const std::string& meshPath) {
	MeshToGPU mesh;

	std::fstream meshFile(meshPath, std::ios::in);
	if (meshFile.is_open()) {
		if (!nlohmann::json::accept(meshFile)) {
			std::cout << "\"" << meshPath << "\" is not a valid JSON file." << std::endl;
			return mesh;
		}
	}
	else {
		std::cout << "\"" << meshPath << "\" cannot be opened." << std::endl;
		return mesh;
	}

	meshFile = std::fstream(meshPath, std::ios::in);

	nlohmann::json j = nlohmann::json::parse(meshFile);

	if (j.contains("vertices")) {
		mesh.vertices.resize(j["vertices"].size() * 8);
		for (size_t i = 0; i < j["vertices"].size(); i++) {
			if (j["vertices"][i].contains("position")) {
				mesh.vertices[(i * 8) + 0] = j["vertices"][i]["position"][0];
				mesh.vertices[(i * 8) + 1] = j["vertices"][i]["position"][1];
				mesh.vertices[(i * 8) + 2] = j["vertices"][i]["position"][2];
			}

			if (j["vertices"][i].contains("normal")) {
				mesh.vertices[(i * 8) + 3] = j["vertices"][i]["normal"][0];
				mesh.vertices[(i * 8) + 4] = j["vertices"][i]["normal"][1];
				mesh.vertices[(i * 8) + 5] = j["vertices"][i]["normal"][2];
			}

			if (j["vertices"][i].contains("uv")) {
				mesh.vertices[(i * 8) + 6] = j["vertices"][i]["uv"][0];
				mesh.vertices[(i * 8) + 7] = j["vertices"][i]["uv"][1];
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

void RendererResourceManager::loadNtim(const std::string& imagePath, const std::string& name) {
	ImageToGPU image;

	std::fstream imageFile(imagePath, std::ios::in);
	if (imageFile.is_open()) {
		if (!nlohmann::json::accept(imageFile)) {
			std::cout << "\"" << imagePath << "\" is not a valid JSON file." << std::endl;
			return;
		}
	}
	else {
		std::cout << "\"" << imagePath << "\" cannot be opened." << std::endl;
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

	if (!image.data.empty()) {
		imagesToGPU[name] = image;
	}
}

void RendererResourceManager::loadGltf(const std::string& modelPath, const std::string& name) {
	ModelToGPU model;
	cgltf_options options = {};
	cgltf_data* data = NULL;
	cgltf_result result = cgltf_parse_file(&options, modelPath.c_str(), &data);
	if (result == cgltf_result_success) {
		result = cgltf_load_buffers(&options, data, modelPath.c_str());

		if (result != cgltf_result_success) {
			std::cout << "Could not load buffers for model file \"" << modelPath << "\"." << std::endl;
		}
		else {
			cgltf_scene* scene = data->scene;

			for (size_t i = 0; i < scene->nodes_count; i++) {
				loadGltfNode(modelPath, model, nml::mat4(), scene->nodes[i]);
			}
		}

		cgltf_free(data);
	}

	if (!model.meshes.empty()) {
		modelsToGPU[name] = model;
	}
}

void RendererResourceManager::loadGltfNode(const std::string& modelPath, ModelToGPU& rendererModel, nml::mat4 modelMatrix, cgltf_node* node) {
	nml::mat4 nodeMatrix;
	if (node->has_matrix) {
		nodeMatrix = nml::mat4(node->matrix);
	}
	else {
		if (node->has_translation) {
			nodeMatrix *= nml::translate(nml::vec3(node->translation));
		}
		if (node->has_rotation) {
			nodeMatrix *= nml::to_mat4(nml::quat(node->rotation[3], node->rotation[0], node->rotation[1], node->rotation[2]));
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

			MeshToGPU primitive;

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
			primitive.vertices.resize(vertexCount * 8);

			size_t positionCursor = 0;
			size_t normalCursor = 0;
			size_t uvCursor = 0;

			for (size_t j = 0; j < vertexCount; j++) {
				nml::vec3 vertexPosition = nml::vec3(modelMatrix * nml::vec4(nml::vec3(position + positionCursor), 1.0f));
				primitive.vertices[(j * 8) + 0] = vertexPosition.x;
				primitive.vertices[(j * 8) + 1] = vertexPosition.y;
				primitive.vertices[(j * 8) + 2] = vertexPosition.z;
				positionCursor += (positionStride / sizeof(float));

				if (normalCount != 0) {
					nml::vec3 vertexNormal = nml::normalize(nml::vec3(nml::transpose(nml::inverse(modelMatrix)) * nml::vec4(nml::vec3(normal + normalCursor), 0.0f)));
					primitive.vertices[(j * 8) + 3] = vertexNormal.x;
					primitive.vertices[(j * 8) + 4] = vertexNormal.y;
					primitive.vertices[(j * 8) + 5] = vertexNormal.z;
					normalCursor += (normalStride / sizeof(float));
				}
				else {
					primitive.vertices[(j * 8) + 3] = 0.0f;
					primitive.vertices[(j * 8) + 4] = 0.0f;
					primitive.vertices[(j * 8) + 5] = 0.0f;
				}

				if (uvCount != 0) {
					primitive.vertices[(j * 8) + 6] = *(uv + uvCursor);
					primitive.vertices[(j * 8) + 7] = *(uv + uvCursor + 1);
					uvCursor += (uvStride / sizeof(float));
				}
				else {
					primitive.vertices[(j * 8) + 6] = 0.0f;
					primitive.vertices[(j * 8) + 7] = 0.0f;
				}
			}

			// Indices
			cgltf_accessor* accessor = nodeMeshPrimitive.indices;
			if (accessor != NULL) {
				primitive.indices.reserve(accessor->count);
				cgltf_buffer_view* bufferView = accessor->buffer_view;
				cgltf_component_type componentType = accessor->component_type;
				std::byte* buffer = static_cast<std::byte*>(bufferView->buffer->data);
				switch (componentType) {
				case cgltf_component_type_r_8:
				{
					int8_t* index = reinterpret_cast<int8_t*>(buffer + accessor->offset + bufferView->offset);
					for (size_t j = 0; j < accessor->count; j++) {
						primitive.indices.push_back(static_cast<uint32_t>(index[j]));
					}
					break;
				}

				case cgltf_component_type_r_8u:
				{
					uint8_t* index = reinterpret_cast<uint8_t*>(buffer + accessor->offset + bufferView->offset);
					for (size_t j = 0; j < accessor->count; j++) {
						primitive.indices.push_back(static_cast<uint32_t>(index[j]));
					}
					break;
				}

				case cgltf_component_type_r_16:
				{
					int16_t* index = reinterpret_cast<int16_t*>(buffer + accessor->offset + bufferView->offset);
					for (size_t j = 0; j < accessor->count; j++) {
						primitive.indices.push_back(static_cast<uint32_t>(index[j]));
					}
					break;
				}

				case cgltf_component_type_r_16u:
				{
					uint16_t* index = reinterpret_cast<uint16_t*>(buffer + accessor->offset + bufferView->offset);
					for (size_t j = 0; j < accessor->count; j++) {
						primitive.indices.push_back(static_cast<uint32_t>(index[j]));
					}
					break;
				}

				case cgltf_component_type_r_32u:
				{
					uint32_t* index = reinterpret_cast<uint32_t*>(buffer + accessor->offset + bufferView->offset);
					std::copy(index, index + accessor->count, std::back_inserter(primitive.indices));
					break;
				}

				case cgltf_component_type_r_32f:
				{
					float* index = reinterpret_cast<float*>(buffer + accessor->offset + bufferView->offset);
					for (size_t j = 0; j < accessor->count; j++) {
						primitive.indices.push_back(static_cast<uint32_t>(index[j]));
					}
					break;
				}

				default:
					std::cout << "Index component type invalid for model file \"" << modelPath << "\"." << std::endl;
				}
			}
			else {
				// Calculate indices
				primitive.indices.resize(primitive.vertices.size());
				std::iota(primitive.indices.begin(), primitive.indices.end(), 0);
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
									std::cout << "Invalid Base64 data when loading glTF embedded texture for model file \"" << modelPath << "\" (base color texture)." << std::endl;
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

							ImageToGPU& image = imagesToGPU[imageURI];
							if (baseColorTexture->sampler != NULL) {
								if ((baseColorTexture->sampler->min_filter == 9728) || (baseColorTexture->sampler->min_filter == 9984) || (baseColorTexture->sampler->min_filter == 9986)) {
									image.minFilter = ImageToGPU::SamplerFilter::Nearest;
								}
								else {
									image.minFilter = ImageToGPU::SamplerFilter::Linear;
								}

								if ((baseColorTexture->sampler->mag_filter == 9728) || (baseColorTexture->sampler->mag_filter == 9984) || (baseColorTexture->sampler->mag_filter == 9986)) {
									image.magFilter = ImageToGPU::SamplerFilter::Nearest;
								}
								else {
									image.magFilter = ImageToGPU::SamplerFilter::Linear;
								}

								if (baseColorTexture->sampler->wrap_s == 10497) {
									image.wrapS = ImageToGPU::SamplerWrap::Repeat;
								}
								else if (baseColorTexture->sampler->wrap_s == 33648) {
									image.wrapS = ImageToGPU::SamplerWrap::MirroredRepeat;
								}

								if (baseColorTexture->sampler->wrap_t == 10497) {
									image.wrapT = ImageToGPU::SamplerWrap::Repeat;
								}
								else if (baseColorTexture->sampler->wrap_t == 33648) {
									image.wrapT = ImageToGPU::SamplerWrap::MirroredRepeat;
								}
							}

							primitive.diffuseTexturePath = imageURI;
						}
					}
					else if (baseColorFactor != NULL) {
						std::string mapKey = "srgb" + std::to_string(baseColorFactor[0]) + std::to_string(baseColorFactor[1]) + std::to_string(baseColorFactor[2]) + std::to_string(baseColorFactor[3]);

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

						primitive.diffuseTexturePath = mapKey;
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
								std::cout << "Invalid Base64 data when loading glTF embedded texture for model file \"" << modelPath << "\" (base color texture)." << std::endl;
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

						ImageToGPU& image = imagesToGPU[imageURI];
						if (emissiveTexture->sampler != NULL) {
							if ((emissiveTexture->sampler->min_filter == 9728) || (emissiveTexture->sampler->min_filter == 9984) || (emissiveTexture->sampler->min_filter == 9986)) {
								image.minFilter = ImageToGPU::SamplerFilter::Nearest;
							}
							else {
								image.minFilter = ImageToGPU::SamplerFilter::Linear;
							}

							if ((emissiveTexture->sampler->mag_filter == 9728) || (emissiveTexture->sampler->mag_filter == 9984) || (emissiveTexture->sampler->mag_filter == 9986)) {
								image.magFilter = ImageToGPU::SamplerFilter::Nearest;
							}
							else {
								image.magFilter = ImageToGPU::SamplerFilter::Linear;
							}

							if (emissiveTexture->sampler->wrap_s == 10497) {
								image.wrapS = ImageToGPU::SamplerWrap::Repeat;
							}
							else if (emissiveTexture->sampler->wrap_s == 33648) {
								image.wrapS = ImageToGPU::SamplerWrap::MirroredRepeat;
							}

							if (emissiveTexture->sampler->wrap_t == 10497) {
								image.wrapT = ImageToGPU::SamplerWrap::Repeat;
							}
							else if (emissiveTexture->sampler->wrap_t == 33648) {
								image.wrapT = ImageToGPU::SamplerWrap::MirroredRepeat;
							}
						}

						primitive.emissiveTexturePath = imageURI;
					}
				}
				else if (emissiveFactor != NULL) {
					std::string mapKey = "srgb" + std::to_string(emissiveFactor[0]) + std::to_string(emissiveFactor[1]) + std::to_string(emissiveFactor[2]) + std::to_string(emissiveFactor[3]);

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

					primitive.emissiveTexturePath = mapKey;
				}
			}

			rendererModel.meshes.push_back(primitive);
		}
	}

	for (size_t i = 0; i < node->children_count; i++) {
		loadGltfNode(modelPath, rendererModel, modelMatrix, node->children[i]);
	}
}

void RendererResourceManager::loadImageStb(const std::string& imagePath, const std::string& name) {
	int width;
	int height;
	int texChannels;

	stbi_uc* pixels = stbi_load(imagePath.c_str(), &width, &height, &texChannels, STBI_rgb_alpha);
	if (!pixels) {
		std::cout << "Could not load image \"" << imagePath << "\"." << std::endl;
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

void RendererResourceManager::loadImageFromMemory(void* data, size_t size, const std::string& name) {
	int width;
	int height;
	int texChannels;

	stbi_uc* pixels = stbi_load_from_memory(reinterpret_cast<stbi_uc*>(data), static_cast<int>(size), &width, &height, &texChannels, STBI_rgb_alpha);
	if (!pixels) {
		std::cout << "Could not load image from memory." << std::endl;
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