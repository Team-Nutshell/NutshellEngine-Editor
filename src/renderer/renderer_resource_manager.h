#pragma once
#include "../renderer/renderer_model.h"
#include "../../external/nml/include/nml.h"
#include "../../external/cgltf/cgltf.h"
#include <qopengl.h>
#include <unordered_map>
#include <vector>
#include <string>
#include <tuple>
#include <cstdint>

class RendererResourceManager {
public:
	void loadModel(const std::string& modelPath);
	void loadImage(const std::string& imagePath);

private:
	RendererModel loadNtmd(const std::string& modelPath);
	RendererMesh loadNtmh(const std::string& modelPath);
	std::tuple<uint32_t, uint32_t, std::vector<uint8_t>> loadNtim(const std::string& modelPath);
	RendererModel loadGltf(const std::string& modelPath);
	void loadGltfNode(const std::string& modelPath, RendererModel& rendererModel, nml::mat4 modelMatrix, cgltf_node* node);

public:
	std::unordered_map<std::string, RendererModel> models;
	std::unordered_map<std::string, int> textures;
};