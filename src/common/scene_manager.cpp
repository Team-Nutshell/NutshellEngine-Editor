#include "scene_manager.h"
#include "../common/save_title_changer.h"
#include "../widgets/main_window.h"
#include "../undo_commands/clear_scene_command.h"
#include "../undo_commands/open_scene_command.h"
#include "../renderer/collider_mesh.h"
#include "../../external/nlohmann/json.hpp"
#include <fstream>

void SceneManager::newScene(GlobalInfo& globalInfo) {
	globalInfo.actionUndoStack->push(new ClearSceneCommand(globalInfo));
	emit globalInfo.signalEmitter.resetCameraSignal();
}

void SceneManager::openScene(GlobalInfo& globalInfo, const std::string& sceneFilePath) {
	std::fstream sceneFile(sceneFilePath, std::ios::in);
	if (sceneFile.is_open()) {
		if (!nlohmann::json::accept(sceneFile)) {
			globalInfo.logger.addLog(LogLevel::Warning, globalInfo.localization.getString("log_file_is_not_valid_json", { sceneFilePath }));
			return;
		}
	}
	else {
		globalInfo.logger.addLog(LogLevel::Warning, globalInfo.localization.getString("log_file_cannot_be_opened", { sceneFilePath }));
		return;
	}
		
	sceneFile = std::fstream(sceneFilePath, std::ios::in);
	nlohmann::json j = nlohmann::json::parse(sceneFile);

	std::vector<Entity> newEntities(j["entities"].size());
	if (j.contains("entities")) {
		for (size_t i = 0; i < j["entities"].size(); i++) {
			Entity newEntity = Entity::fromJson(j["entities"][i]);
			newEntity.entityID = globalInfo.globalEntityID++;
			newEntities[i] = newEntity;
			if (newEntity.renderable) {
				if (!newEntity.renderable->modelPath.empty()) {
					std::string fullModelPath = newEntity.renderable->modelPath;
					std::filesystem::path path(fullModelPath);
					if (!path.is_absolute()) {
						if (std::filesystem::exists(globalInfo.projectDirectory + "/" + fullModelPath)) {
							fullModelPath = std::filesystem::canonical(globalInfo.projectDirectory + "/" + fullModelPath).string();
						}
					}
					std::replace(fullModelPath.begin(), fullModelPath.end(), '\\', '/');
					if (std::filesystem::exists(fullModelPath)) {
						globalInfo.rendererResourceManager.loadModel(fullModelPath, newEntity.renderable->modelPath);
					}
				}

				if (!newEntity.renderable->materialPath.empty()) {
					std::string fullMaterialPath = newEntity.renderable->materialPath;
					std::filesystem::path path(fullMaterialPath);
					if (!path.is_absolute()) {
						if (std::filesystem::exists(globalInfo.projectDirectory + "/" + fullMaterialPath)) {
							fullMaterialPath = std::filesystem::canonical(globalInfo.projectDirectory + "/" + fullMaterialPath).string();
						}
					}
					std::replace(fullMaterialPath.begin(), fullMaterialPath.end(), '\\', '/');
					if (std::filesystem::exists(fullMaterialPath)) {
						globalInfo.rendererResourceManager.loadMaterial(fullMaterialPath, newEntity.renderable->materialPath);
					}
				}
			}
		}
	}
	globalInfo.actionUndoStack->push(new OpenSceneCommand(globalInfo, newEntities, sceneFilePath));

	for (const auto& newEntity : newEntities) {
		ColliderMesh::update(globalInfo, newEntity.entityID);
	}
}

void SceneManager::saveScene(GlobalInfo& globalInfo, const std::string& sceneFilePath) {
	globalInfo.currentScenePath = sceneFilePath;
	globalInfo.mainWindow->updateTitle();
	nlohmann::json j;
	for (int i = 0; i < globalInfo.mainWindow->entityPanel->entityList->count(); i++) {
		EntityListItem* entityListItem = static_cast<EntityListItem*>(globalInfo.mainWindow->entityPanel->entityList->item(i));

		j["entities"].push_back(globalInfo.entities[entityListItem->entityID].toJson());
	}

	std::fstream sceneFile(sceneFilePath, std::ios::out | std::ios::trunc);
	if (j.empty()) {
		sceneFile << "{\n}";
	}
	else {
		sceneFile << j.dump(1, '\t');
	}

	SaveTitleChanger::reset(globalInfo.mainWindow);
}