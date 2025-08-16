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

enum class SelectionType {
	Asset,
	Entities
};

struct GlobalInfo {
	std::string version;

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

	std::unique_ptr<QUndoStack> actionUndoStack;
	std::unique_ptr<QUndoStack> selectionUndoStack;
	SelectionType lastSelectionType = SelectionType::Entities;
	EditorParameters editorParameters;
	SignalEmitter signalEmitter;
	Localization localization;
	Logger logger;
	RendererResourceManager rendererResourceManager = RendererResourceManager(&localization, &logger);

	uint32_t steamAppID = 0;

	EntityID findEntityByName(const std::string& entityName) {
		for (const auto& entity : entities) {
			if (entity.second.name == entityName) {
				return entity.first;
			}
		}

		return NO_ENTITY;
	}
};