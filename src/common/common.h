#pragma once
#include "entity.h"
#include "signal_emitter.h"
#include "../renderer/renderer_resource_manager.h"
#include "../../external/nml/include/nml.h"
#include <QUndoStack>
#include <memory>
#include <unordered_map>
#include <algorithm>
#include <string>
#include <cstdint>

struct GlobalInfo {
	std::string projectDirectory = "";
	std::string projectName = "";
	std::unordered_map<EntityID, Entity> entities;
	EntityID currentEntityID = NO_ENTITY;
	EntityID globalEntityID = 0;
	EntityID copiedEntityID = NO_ENTITY;
	std::string currentScenePath = "";
	float devicePixelRatio = 1.0f;
	void* mainWindow;
	std::unique_ptr<QUndoStack> undoStack;
	SignalEmitter signalEmitter;
	RendererResourceManager rendererResourceManager;

	EntityID findEntityByName(const std::string& entityName) {
		for (const auto& entity : entities) {
			if (entity.second.name == entityName) {
				return entity.first;
			}
		}

		return NO_ENTITY;
	}
};