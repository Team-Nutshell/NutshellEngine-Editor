#pragma once
#include "../renderer/renderer_resources.h"
#include "../../external/nml/include/nml.h"
#include "../../external/cgltf/cgltf.h"
#include <unordered_map>
#include <vector>
#include <string>
#include <tuple>
#include <filesystem>
#include <cstdint>

class Localization;
class Logger;

class RendererResourceManager {
public:
	enum class AssetType {
		Model,
		Material,
		Image,
		ImageSampler,
		Unknown
	};

	struct Mesh {
		struct Vertex {
			nml::vec3 position;
			nml::vec3 normal;
			nml::vec2 uv;
		};

		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		bool collidersCalculated = false;
		OBB obb;
		Sphere sphere;
		Capsule capsule;
	};

	struct SamplerToGPU {
		enum class Filter {
			Nearest,
			Linear
		};

		enum class Wrap {
			ClampToEdge,
			Repeat,
			MirroredRepeat
		};

		static std::string filterToString(Filter filter) {
			switch (filter) {
			case Filter::Nearest:
				return "Nearest";
			case Filter::Linear:
				return "Linear";
			default:
				return "Nearest";
			}
		}

		static std::string wrapToString(Wrap wrap) {
			switch (wrap) {
			case Wrap::ClampToEdge:
				return "ClampToEdge";
			case Wrap::Repeat:
				return "Repeat";
			case Wrap::MirroredRepeat:
				return "MirroredRepeat";
			default:
				return "ClampToEdge";
			}
		}

		std::string toString() {
			return "mag:" + SamplerToGPU::filterToString(magFilter) +
				"/min:" + SamplerToGPU::filterToString(minFilter) +
				"/mip:" + SamplerToGPU::filterToString(mipmapFilter) +
				"/wS:" + SamplerToGPU::wrapToString(wrapS) +
				"/wT:" + SamplerToGPU::wrapToString(wrapT);
		}

		Filter magFilter = Filter::Nearest;
		Filter minFilter = Filter::Nearest;
		Filter mipmapFilter = Filter::Nearest;
		Wrap wrapS = Wrap::ClampToEdge;
		Wrap wrapT = Wrap::ClampToEdge;
	};

	struct Material {
		std::string diffuseTextureName = "defaultDiffuseTexture";
		std::string diffuseTextureSamplerName = "defaultSampler";
		std::string emissiveTextureName = "defaultEmissiveTexture";
		std::string emissiveTextureSamplerName = "defaultSampler";
		float emissiveFactor = 1.0f;
		float alphaCutoff = 0.0f;
		bool useTriplanarMapping = false;
		nml::vec2 scaleUV = nml::vec2(1.0f, 1.0f);
		nml::vec2 offsetUV = nml::vec2(0.0f, 0.0f);
	};

	struct ModelPrimitive {
		std::string name = "";
		nml::mat4 modelMatrix = nml::mat4::identity();
		Mesh mesh;
		Material material;
	};

	struct Model {
		std::vector<ModelPrimitive> primitives;
	};

	struct ImageToGPU {
		uint32_t width = 0;
		uint32_t height = 0;

		std::vector<uint8_t> data;
	};

public:
	RendererResourceManager(Localization* passLocalization, Logger* passLogger);

public:
	void loadModel(const std::string& modelPath, const std::string& name);
	void loadMaterial(const std::string& materialPath, const std::string& name);
	void loadImage(const std::string& imagePath, const std::string& name);
	void loadSampler(const std::string& samplerPath, const std::string& name);

	void loadMeshColliders(Mesh& mesh);

	AssetType getFileAssetType(const std::string& path);

private:
	void loadNtmd(const std::string& modelPath, Model& model);
	Mesh loadNtmh(const std::string& meshPath);
	void loadNtml(const std::string& materialPath, Material& material);
	void loadNtim(const std::string& imagePath, ImageToGPU& image);
	void loadNtsp(const std::string& samplerPath, SamplerToGPU& sampler);
	void loadGltf(const std::string& modelPath, Model& model);
	void loadGltfNode(const std::string& modelPath, Model& rendererModel, nml::mat4 modelMatrix, cgltf_node* node);
	void loadImageStb(const std::string& imagePath, ImageToGPU& image);
	void loadImageFromMemory(void* data, size_t size, const std::string& name);
	void loadObj(const std::string& modelPath, Model& model);
	std::unordered_map<std::string, Material> loadMtl(const std::string& materialPath);

public:
	std::unordered_map<std::string, RendererModel> rendererModels;
	std::unordered_map<std::string, uint32_t> textures;
	std::unordered_map<std::string, RendererSampler> samplers;

	std::unordered_map<std::string, Model> models;
	std::unordered_map<std::string, Material> materials;
	std::unordered_map<std::string, ImageToGPU> imagesToGPU;
	std::unordered_map<std::string, SamplerToGPU> samplersToGPU;

	std::vector<std::string> modelsToLoad;

	std::unordered_map<std::string, std::filesystem::file_time_type> modelLastWriteTime;
	std::unordered_map<std::string, std::filesystem::file_time_type> materialLastWriteTime;
	std::unordered_map<std::string, std::filesystem::file_time_type> imageLastWriteTime;
	std::unordered_map<std::string, std::filesystem::file_time_type> samplerLastWriteTime;

	std::unordered_map<std::string, std::vector<std::string>> modelNtmdPrimitiveToMaterialPath;

	std::string projectDirectory = "";

	Localization* localization;
	Logger* logger;
};