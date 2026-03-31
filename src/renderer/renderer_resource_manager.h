#pragma once
#include "../renderer/renderer_resources.h"
#include "../../external/nml/include/nml.h"
#include "../../external/cgltf/cgltf.h"
#include <unordered_map>
#include <vector>
#include <set>
#include <string>
#include <tuple>
#include <filesystem>
#include <limits>
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
			nml::vec3 color;
			nml::vec4 tangent;
		};

		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		bool collidersCalculated = false;
		OBB obb;
		Sphere sphere;
		Capsule capsule;

		nml::vec3 aabbMin = nml::vec3(std::numeric_limits<float>::max());
		nml::vec3 aabbMax = nml::vec3(std::numeric_limits<float>::lowest());
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
			return "mag:" + filterToString(magFilter) +
				"/min:" + filterToString(minFilter) +
				"/mip:" + filterToString(mipmapFilter) +
				"/wS:" + wrapToString(wrapS) +
				"/wT:" + wrapToString(wrapT) +
				"/aL:" + std::to_string(anisotropyLevel);
		}

		Filter magFilter = Filter::Nearest;
		Filter minFilter = Filter::Nearest;
		Filter mipmapFilter = Filter::Nearest;
		Wrap wrapS = Wrap::ClampToEdge;
		Wrap wrapT = Wrap::ClampToEdge;
		float anisotropyLevel = 0.0f;
	};

	struct ModelPrimitive {
		std::string name = "";
		nml::mat4 modelMatrix = nml::mat4::identity();
		Mesh mesh;
		RendererMaterial material;
	};

	struct Model {
		std::vector<ModelPrimitive> primitives;
	};

	struct ImageToGPU {
		enum class ColorSpace {
			Linear,
			SRGB
		};

		uint32_t width = 0;
		uint32_t height = 0;
		ColorSpace colorSpace = ColorSpace::Linear;

		std::vector<uint8_t> data;
	};

public:
	RendererResourceManager(Localization* passLocalization, Logger* passLogger);

public:
	void loadModel(const std::string& modelPath, const std::string& name);
	void loadMaterial(const std::string& materialPath, const std::string& name);
	void loadImage(const std::string& imagePath, const std::string& name);
	void loadSampler(const std::string& samplerPath, const std::string& name);
	void loadFragmentShader(const std::string& fragmentShaderPath, const std::string& name);

	void loadMeshColliders(Mesh& mesh);

private:
	void loadNtmd(const std::string& modelPath, Model& model);
	Mesh loadNtmh(const std::string& meshPath);
	void loadNtml(const std::string& materialPath, RendererMaterial& material);
	void loadNtim(const std::string& imagePath, ImageToGPU& image);
	void loadNtsp(const std::string& samplerPath, SamplerToGPU& sampler);
	void loadGltf(const std::string& modelPath, Model& model);
	void loadGltfNode(const std::string& modelPath, Model& rendererModel, nml::mat4 modelMatrix, cgltf_node* node);
	void loadImageStb(const std::string& imagePath, ImageToGPU& image);
	void loadImageFromMemory(void* data, size_t size, const std::string& name);
	void loadObj(const std::string& modelPath, Model& model);
	std::unordered_map<std::string, RendererMaterial> loadMtl(const std::string& materialPath);

	void calculateTangents(Mesh& mesh);

public:
	std::unordered_map<std::string, RendererModel> rendererModels;
	std::unordered_map<std::string, RendererMaterial> materials;
	std::unordered_map<std::string, uint32_t> textures;
	std::unordered_map<std::string, RendererSampler> samplers;
	std::unordered_map<std::string, GLuint> fragmentShaders;
	std::unordered_map<std::string, GLuint> fragmentShaderPrograms;

	std::unordered_map<std::string, Model> models;
	std::unordered_map<std::string, ImageToGPU> imagesToGPU;
	std::unordered_map<std::string, SamplerToGPU> samplersToGPU;
	std::unordered_map<std::string, std::string> fragmentShadersToGPU;

	std::vector<std::string> modelsToLoad;

	std::unordered_map<std::string, std::filesystem::file_time_type> modelLastWriteTimes;
	std::unordered_map<std::string, std::filesystem::file_time_type> materialLastWriteTimes;
	std::unordered_map<std::string, std::filesystem::file_time_type> imageLastWriteTimes;
	std::unordered_map<std::string, std::filesystem::file_time_type> samplerLastWriteTimes;
	std::unordered_map<std::string, std::filesystem::file_time_type> fragmentShaderLastWriteTimes;

	std::unordered_map<std::string, std::vector<std::string>> modelNtmdPrimitiveToMaterialPaths;

	std::string projectDirectory = "";

	Localization* localization;
	Logger* logger;
};