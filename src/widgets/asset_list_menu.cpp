#include "asset_list_menu.h"
#include "rename_widget.h"
#include "main_window.h"
#include <filesystem>
#include <fstream>

AssetListMenu::AssetListMenu(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	renameAction = addAction("Rename", this, &AssetListMenu::rename);
	addSeparator();
	QMenu* createMenu = addMenu("Create");
	newDirectoryAction = createMenu->addAction("Directory", this, &AssetListMenu::newDirectory);
	createMenu->addSeparator();
	newModelAction = createMenu->addAction("Model", this, &AssetListMenu::newModel);
	newImageSamplerAction = createMenu->addAction("Image Sampler", this, &AssetListMenu::newImageSampler);
	newMaterialAction = createMenu->addAction("Material", this, &AssetListMenu::newMaterial);
	newSceneAction = createMenu->addAction("Scene", this, &AssetListMenu::newScene);
}

void AssetListMenu::rename() {
	RenameWidget* renameWidget = new RenameWidget(m_globalInfo, directory, filename);
	renameWidget->show();
	connect(renameWidget, &RenameWidget::renameFileSignal, this, &AssetListMenu::onFileRenamed);
}

void AssetListMenu::newDirectory() {
	std::string baseNewDirectoryName = "directory";
	std::string newDirectoryName = baseNewDirectoryName;
	uint32_t directoryNameIndex = 0;
	while (std::filesystem::exists(directory + "/" + newDirectoryName)) {
		newDirectoryName = baseNewDirectoryName + "_" + std::to_string(directoryNameIndex);
		directoryNameIndex++;
	}
	std::filesystem::create_directory(directory + "/" + newDirectoryName);
}

void AssetListMenu::newModel() {
	std::string baseNewModelName = "model";
	std::string modelExtension = ".ntmd";
	std::string newModelName = baseNewModelName;
	uint32_t modelNameIndex = 0;
	while (std::filesystem::exists(directory + "/" + newModelName + modelExtension)) {
		newModelName = baseNewModelName + "_" + std::to_string(modelNameIndex);
		modelNameIndex++;
	}
	std::ofstream newModelFile(directory + "/" + newModelName + modelExtension);
	newModelFile << R"({
})";
	newModelFile.close();
}

void AssetListMenu::newImageSampler() {
	std::string baseNewImageSamplerName = "image_sampler";
	std::string imageSamplerExtension = ".ntsp";
	std::string newImageSamplerName = baseNewImageSamplerName;
	uint32_t imageSamplerNameIndex = 0;
	while (std::filesystem::exists(directory + "/" + newImageSamplerName + imageSamplerExtension)) {
		newImageSamplerName = baseNewImageSamplerName + "_" + std::to_string(imageSamplerNameIndex);
		imageSamplerNameIndex++;
	}
	std::ofstream newImageSamplerFile(directory + "/" + newImageSamplerName + imageSamplerExtension);
	newImageSamplerFile << R"({
	"magFilter": "Nearest",
	"minFilter": "Nearest",
	"mipmapFilter": "Nearest",
	"addressModeU": "ClampToEdge",
	"addressModeV": "ClampToEdge",
	"addressModeW": "ClampToEdge",
	"borderColor": "FloatOpaqueBlack",
	"anisotropyLevel": 0
})";
	newImageSamplerFile.close();
}

void AssetListMenu::newMaterial() {
	std::string baseNewMaterialName = "material";
	std::string materialExtension = ".ntml";
	std::string newMaterialName = baseNewMaterialName;
	uint32_t materialNameIndex = 0;
	while (std::filesystem::exists(directory + "/" + newMaterialName + materialExtension)) {
		newMaterialName = baseNewMaterialName + "_" + std::to_string(materialNameIndex);
		materialNameIndex++;
	}
	std::ofstream newMaterialFile(directory + "/" + newMaterialName + materialExtension);
	newMaterialFile << R"({
	"diffuse": {
		"color": [
			0.5,
			0.5,
			0.5,
			1.0
		]
	},
	"metalness": {
		"value": 0.5
	},
	"roughness": {
		"value": 0.5
	},
	"occlusion": {
		"value": 1.0
	},
	"emissive": {
		"color": [
			0.0,
			0.0,
			0.0
		],
		"factor": 1.0
	},
	"alphaCutoff": 0.0,
	"indexOfRefraction": 0.0
})";
	newMaterialFile.close();
}

void AssetListMenu::newScene() {
	std::string baseNewSceneName = "scene";
	std::string sceneExtension = ".ntsn";
	std::string newSceneName = baseNewSceneName;
	uint32_t sceneNameIndex = 0;
	while (std::filesystem::exists(directory + "/" + newSceneName + sceneExtension)) {
		newSceneName = baseNewSceneName + "_" + std::to_string(sceneNameIndex);
		sceneNameIndex++;
	}
	std::ofstream newSceneFile(directory + "/" + newSceneName + sceneExtension);
	newSceneFile << R"({
})";
	newSceneFile.close();
}

void AssetListMenu::onFileRenamed(const std::string& oldFilename, const std::string& newFilename) {
	if (oldFilename == m_globalInfo.currentScenePath) {
		m_globalInfo.currentScenePath = newFilename;
		m_globalInfo.mainWindow->updateTitle();
	}
}
