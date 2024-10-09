#include "asset_list_menu.h"
#include "asset_info_name_widget.h"
#include "delete_asset_widget.h"
#include "main_window.h"
#include "../common/asset_helper.h"
#include <filesystem>
#include <fstream>

AssetListMenu::AssetListMenu(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	renameAction = addAction(QString::fromStdString(m_globalInfo.localization.getString("assets_rename")), this, &AssetListMenu::renameAsset);
	deleteAction = addAction(QString::fromStdString(m_globalInfo.localization.getString("assets_delete")), this, &AssetListMenu::deleteAsset);
	addSeparator();
	reloadAction = addAction(QString::fromStdString(m_globalInfo.localization.getString("assets_reload")), this, &AssetListMenu::reloadAsset);
	addSeparator();
	QMenu* createMenu = addMenu(QString::fromStdString(m_globalInfo.localization.getString("assets_create")));
	newDirectoryAction = createMenu->addAction(QString::fromStdString(m_globalInfo.localization.getString("directory")), this, &AssetListMenu::newDirectory);
	createMenu->addSeparator();
	newModelAction = createMenu->addAction(QString::fromStdString(m_globalInfo.localization.getString("model")), this, &AssetListMenu::newModel);
	newImageSamplerAction = createMenu->addAction(QString::fromStdString(m_globalInfo.localization.getString("image_sampler")), this, &AssetListMenu::newImageSampler);
	newMaterialAction = createMenu->addAction(QString::fromStdString(m_globalInfo.localization.getString("material")), this, &AssetListMenu::newMaterial);
	newSceneAction = createMenu->addAction(QString::fromStdString(m_globalInfo.localization.getString("scene")), this, &AssetListMenu::newScene);
}

void AssetListMenu::renameAsset() {
	QListWidgetItem* item = m_globalInfo.mainWindow->resourcePanel->assetList->selectedItems()[0];
	m_globalInfo.mainWindow->resourcePanel->assetList->currentlyEditedItemName = item->text().toStdString();
	item->setFlags(item->flags() | Qt::ItemFlag::ItemIsEditable);
	m_globalInfo.mainWindow->resourcePanel->assetList->editItem(item);
}

void AssetListMenu::deleteAsset() {
	DeleteAssetWidget* deleteAssetWidget = new DeleteAssetWidget(m_globalInfo, directory + "/" + filename);
	deleteAssetWidget->show();
}

void AssetListMenu::reloadAsset() {
	std::string assetPath = directory + "/" + m_globalInfo.mainWindow->resourcePanel->assetList->selectedItems()[0]->text().toStdString();
	std::string assetName = AssetHelper::absoluteToRelative(assetPath, m_globalInfo.projectDirectory);
	size_t lastDot = assetName.rfind('.');
	if (lastDot != std::string::npos) {
		std::string extension = assetName.substr(lastDot + 1);
		RendererResourceManager::AssetType assetType = m_globalInfo.rendererResourceManager.getFileAssetType(assetPath);
		if (assetType == RendererResourceManager::AssetType::Model) {
			m_globalInfo.rendererResourceManager.loadModel(assetPath, assetName);
		}
		else if (assetType == RendererResourceManager::AssetType::Material) {
			m_globalInfo.rendererResourceManager.loadMaterial(assetPath, assetName);
		}
		else if (assetType == RendererResourceManager::AssetType::Image) {
			m_globalInfo.rendererResourceManager.loadImage(assetPath, assetName);
		}
		else if (assetType == RendererResourceManager::AssetType::ImageSampler) {
			m_globalInfo.rendererResourceManager.loadSampler(assetPath, assetName);
		}
	}
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