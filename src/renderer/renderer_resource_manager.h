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

class Logger;

class RendererResourceManager {
public:
	struct MeshToGPU {
		struct Vertex {
			nml::vec3 position;
			nml::vec3 normal;
			nml::vec2 uv;
		};

		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

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

	struct MaterialToGPU {
		std::string diffuseTextureName;
		std::string diffuseTextureSamplerName;
		std::string emissiveTextureName;
		std::string emissiveTextureSamplerName;
		float alphaCutoff = 0.0f;
	};

	struct PrimitiveToGPU {
		MeshToGPU mesh;
		MaterialToGPU material;
	};

	struct ModelToGPU {
		std::vector<PrimitiveToGPU> primitives;
	};

	struct ImageToGPU {
		uint32_t width = 0;
		uint32_t height = 0;

		std::vector<uint8_t> data;
	};

public:
	RendererResourceManager(Logger* passLogger);

public:
	void loadModel(const std::string& modelPath, const std::string& name);
	void loadImage(const std::string& imagePath, const std::string& name);
	void loadSampler(const std::string& samplerPath, const std::string& name);

private:
	void loadNtmd(const std::string& modelPath, const std::string& name);
	MeshToGPU loadNtmh(const std::string& meshPath);
	void loadNtim(const std::string& imagePath, const std::string& name);
	void loadNtsp(const std::string& samplerPath, const std::string& name);
	void loadGltf(const std::string& modelPath, const std::string& name);
	void loadGltfNode(const std::string& modelPath, ModelToGPU& rendererModel, nml::mat4 modelMatrix, cgltf_node* node);
	void loadImageStb(const std::string& imagePath, const std::string& name);
	void loadImageFromMemory(void* data, size_t size, const std::string& name);

public:
	std::unordered_map<std::string, RendererModel> models;
	std::unordered_map<std::string, uint32_t> textures;
	std::unordered_map<std::string, RendererSampler> samplers;

	std::unordered_map<std::string, ModelToGPU> modelsToGPU;
	std::unordered_map<std::string, ImageToGPU> imagesToGPU;
	std::unordered_map<std::string, SamplerToGPU> materialsToGPU;
	std::unordered_map<std::string, SamplerToGPU> samplersToGPU;

	std::unordered_map<std::string, std::filesystem::file_time_type> resourceLastWriteTime;

	std::string projectDirectory = "";

	Logger* logger;
};