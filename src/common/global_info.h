#pragma once
#include "editor_parameters.h"
#include "entity.h"
#include "localization.h"
#include "logger.h"
#include "signal_emitter.h"
#include "../renderer/renderer_resource_manager.h"
#include "../../external/nml/include/nml.h"
#include <QUndoStack>
#include <memory>
#include <unordered_map>
#include <set>
#include <algorithm>
#include <string>
#include <vector>
#include <cstdint>

class MainWindow;

struct GlobalInfo {
	std::string version = "0.1.5";

	std::string projectDirectory = "";
	std::string projectName = "";

	std::unordered_map<EntityID, Entity> entities;
	EntityID currentEntityID = NO_ENTITY;
	std::set<EntityID> otherSelectedEntityIDs;
	EntityID globalEntityID = 0;
	std::vector<Entity> copiedEntities;

	std::string currentScenePath = "";

	float devicePixelRatio = 1.0f;

	MainWindow* mainWindow;

	std::unique_ptr<QUndoStack> undoStack;
	EditorParameters editorParameters;
	SignalEmitter signalEmitter;
	Localization localization;
	Logger logger;
	RendererResourceManager rendererResourceManager = RendererResourceManager(&localization, &logger);

	EntityID findEntityByName(const std::string& entityName) {
		for (const auto& entity : entities) {
			if (entity.second.name == entityName) {
				return entity.first;
			}
		}

		return NO_ENTITY;
	}

	void clearSelectedEntities() {
		currentEntityID = NO_ENTITY;
		otherSelectedEntityIDs.clear();
		emit signalEmitter.selectEntitySignal();
	}
};