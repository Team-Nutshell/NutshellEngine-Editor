#pragma once
#include "entity.h"
#include "signal_emitter.h"
#include "../renderer/renderer_resource_manager.h"
#include "../../external/nml/include/nml.h"
#include <QUndoStack>
#include <memory>
#include <vector>
#include <algorithm>
#include <string>
#include <cstdint>

struct GlobalInfo {
	std::string projectDirectory = "";
	std::string projectName = "";
	std::vector<Entity> entities;
	EntityID currentEntityID = NO_ENTITY;
	EntityID globalEntityID = 0;
	EntityID copiedEntityID = NO_ENTITY;
	std::string currentScenePath = "";
	float devicePixelRatio = 1.0f;
	void* mainWindow;
	std::unique_ptr<QUndoStack> undoStack;
	SignalEmitter signalEmitter;
	RendererResourceManager rendererResourceManager;

	uint32_t findEntityById(EntityID entityID) {
		std::vector<Entity>::const_iterator it = std::find_if(entities.begin(), entities.end(), [this, entityID](const Entity& entity) {
			return entity.entityID == entityID;
			});
		
		if (it == entities.end()) {
			return NO_ENTITY;
		}
		
		return std::distance(entities.cbegin(), it);
	}

	uint32_t findCurrentEntity() {
		std::vector<Entity>::const_iterator it = std::find_if(entities.begin(), entities.end(), [this](const Entity& entity) {
			return entity.entityID == currentEntityID;
			});

		if (it == entities.end()) {
			return NO_ENTITY;
		}

		return std::distance(entities.cbegin(), it);
	}

	uint32_t findEntityByName(const std::string& entityName) {
		std::vector<Entity>::const_iterator it = std::find_if(entities.begin(), entities.end(), [this, entityName](const Entity& entity) {
			return entity.name == entityName;
			});

		if (it == entities.end()) {
			return NO_ENTITY;
		}

		return std::distance(entities.cbegin(), it);
	}
};