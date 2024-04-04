#pragma once
#include "common.h"
#include "../widgets/main_window.h"
#include "../undo_commands/clear_scene_command.h"
#include "../undo_commands/open_scene_command.h"
#include "../../external/nlohmann/json.hpp"
#include <fstream>
#include <string>

struct SceneManager {
	static void newScene(GlobalInfo& globalInfo) {
		globalInfo.currentScenePath = "";
		MainWindow* mainWindow = reinterpret_cast<MainWindow*>(globalInfo.mainWindow);
		mainWindow->setWindowTitle("NutshellEngine - " + QString::fromStdString(globalInfo.projectName));
		globalInfo.undoStack->push(new ClearSceneCommand(globalInfo));
		emit globalInfo.signalEmitter.resetCameraSignal();
	}

	static void openScene(GlobalInfo& globalInfo, const std::string& sceneFilePath) {
		std::fstream sceneFile(sceneFilePath, std::ios::in);
		if (sceneFile.is_open()) {
			if (!nlohmann::json::accept(sceneFile)) {
				std::cout << "\"" << sceneFilePath << "\" is not a valid JSON file." << std::endl;
				return;
			}
		}
		else {
			std::cout << "\"" << sceneFilePath << "\" cannot be opened." << std::endl;
			return;
		}
		
		sceneFile = std::fstream(sceneFilePath, std::ios::in);
		nlohmann::json j = nlohmann::json::parse(sceneFile);

		SceneManager::newScene(globalInfo);
		globalInfo.currentScenePath = sceneFilePath;
		MainWindow* mainWindow = reinterpret_cast<MainWindow*>(globalInfo.mainWindow);
		mainWindow->setWindowTitle("NutshellEngine - " + QString::fromStdString(globalInfo.projectName) + " - " + QString::fromStdString(sceneFilePath));
		if (j.contains("entities")) {
			std::unordered_map<EntityID, Entity> entities;
			for (size_t i = 0; i < j["entities"].size(); i++) {
				Entity newEntity = Entity::fromJson(j["entities"][i]);
				newEntity.entityID = globalInfo.globalEntityID++;
				entities[newEntity.entityID] = newEntity;
				if (newEntity.renderable && (newEntity.renderable->modelPath != "")) {
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
			}
			if (!entities.empty()) {
				globalInfo.undoStack->push(new OpenSceneCommand(globalInfo, entities));
			}
		}
	}

	static void saveScene(GlobalInfo& globalInfo, const std::string& sceneFilePath) {
		globalInfo.currentScenePath = sceneFilePath;
		MainWindow* mainWindow = reinterpret_cast<MainWindow*>(globalInfo.mainWindow);
		mainWindow->setWindowTitle("NutshellEngine - " + QString::fromStdString(globalInfo.projectName) + " - " + QString::fromStdString(sceneFilePath));
		std::fstream sceneFile(sceneFilePath, std::ios::out | std::ios::trunc);
		nlohmann::json j;
		for (const auto& entity : globalInfo.entities) {
			j["entities"].push_back(entity.second.toJson());
		}
		sceneFile << j.dump(1, '\t');
	}
};