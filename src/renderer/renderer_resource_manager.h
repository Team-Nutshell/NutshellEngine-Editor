#pragma once
#include "../renderer/renderer_model.h"
#include "../../external/nml/include/nml.h"
#include "../../external/cgltf/cgltf.h"
#include <unordered_map>
#include <vector>
#include <string>
#include <tuple>
#include <cstdint>

class Logger;

class RendererResourceManager {
public:

	struct MeshToGPU {
		struct Vertex {
			nml::vec3 position;
			nml::vec3 normal;
			nml::vec2 uv;
		};

		struct OBB {
			nml::vec3 center;
			nml::vec3 halfExtent;
			nml::vec3 rotation;
		};

		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		std::string diffuseTexturePath;
		std::string emissiveTexturePath;
		float alphaCutoff = 0.0f;

		OBB obb;
	};

	struct ModelToGPU {
		std::vector<MeshToGPU> meshes;
	};

	struct ImageToGPU {
		enum class SamplerFilter {
			Nearest,
			Linear
		};

		enum class SamplerWrap {
			ClampToEdge,
			Repeat,
			MirroredRepeat
		};

		uint32_t width = 0;
		uint32_t height = 0;

		SamplerFilter minFilter = SamplerFilter::Nearest;
		SamplerFilter magFilter = SamplerFilter::Nearest;
		SamplerFilter mipmapFilter = SamplerFilter::Nearest;
		SamplerWrap wrapS = SamplerWrap::ClampToEdge;
		SamplerWrap wrapT = SamplerWrap::ClampToEdge;

		std::vector<uint8_t> data;
	};

public:
	RendererResourceManager(Logger* passLogger);

public:
	void loadModel(const std::string& modelPath, const std::string& name);
	void loadImage(const std::string& imagePath, const std::string& name);

private:
	void loadNtmd(const std::string& modelPath, const std::string& name);
	MeshToGPU loadNtmh(const std::string& meshPath);
	void loadNtim(const std::string& imagePath, const std::string& name);
	void loadGltf(const std::string& modelPath, const std::string& name);
	void loadGltfNode(const std::string& modelPath, ModelToGPU& rendererModel, nml::mat4 modelMatrix, cgltf_node* node);
	void loadImageStb(const std::string& imagePath, const std::string& name);
	void loadImageFromMemory(void* data, size_t size, const std::string& name);

public:
	std::unordered_map<std::string, RendererModel> models;
	std::unordered_map<std::string, uint32_t> textures;

	std::unordered_map<std::string, ModelToGPU> modelsToGPU;
	std::unordered_map<std::string, ImageToGPU> imagesToGPU;

	std::string projectDirectory = "";

	Logger* logger;
};