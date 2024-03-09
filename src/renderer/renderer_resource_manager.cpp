#include "renderer_resource_manager.h"
#pragma warning(push)
#pragma warning(disable : 4996)
#define CGLTF_IMPLEMENTATION
#include "../../external/cgltf/cgltf.h"
#pragma warning(pop)
#define STB_IMAGE_IMPLEMENTATION
#include "../../external/stb/stb_image.h"

void RendererResourceManager::loadModel(const std::string& modelPath) {
	(void)modelPath;
}

void RendererResourceManager::loadImage(const std::string& imagePath) {
	(void)imagePath;
}

RendererModel RendererResourceManager::loadNtmd(const std::string& modelPath) {
	(void)modelPath;
	return RendererModel();
}

RendererMesh RendererResourceManager::loadNtmh(const std::string& modelPath) {
	(void)modelPath;
	return RendererMesh();
}

std::tuple<uint32_t, uint32_t, std::vector<uint8_t>> RendererResourceManager::loadNtim(const std::string& modelPath) {
	(void)modelPath;
	return std::tuple<uint32_t, uint32_t, std::vector<uint8_t>>();
}

RendererModel RendererResourceManager::loadGltf(const std::string& modelPath) {
	(void)modelPath;
	return RendererModel();
}

void RendererResourceManager::loadGltfNode(const std::string& modelPath, RendererModel& rendererModel, nml::mat4 modelMatrix, cgltf_node* node) {
	(void)modelPath;
	(void)rendererModel;
	(void)modelMatrix;
	(void)node;
}
