#include "open_scene_command.h"

OpenSceneCommand::OpenSceneCommand(GlobalInfo& globalInfo, const std::unordered_map<EntityID, Entity>& entities) : m_globalInfo(globalInfo) {
	setText("Open Scene");
	m_newEntities = entities;
}

void OpenSceneCommand::undo() {
	while (!m_globalInfo.entities.empty()) {
		EntityID destroyedEntityID = m_globalInfo.entities.begin()->first;
		m_globalInfo.entities.erase(destroyedEntityID);
		emit m_globalInfo.signalEmitter.destroyEntitySignal(destroyedEntityID);
	}
}

void OpenSceneCommand::redo() {
	m_globalInfo.entities = m_newEntities;
	for (const auto& newEntity : m_newEntities) {
		emit m_globalInfo.signalEmitter.createEntitySignal(newEntity.first);
	}
}