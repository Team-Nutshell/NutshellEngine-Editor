#include "open_scene_command.h"

OpenSceneCommand::OpenSceneCommand(GlobalInfo& globalInfo, const std::unordered_map<EntityID, Entity>& entities, const std::string& scenePath) : m_globalInfo(globalInfo) {
	setText("Open Scene \"" + QString::fromStdString(scenePath) + "\"");
	m_newEntities = entities;
}

void OpenSceneCommand::undo() {
	while (!m_globalInfo.entities.empty()) {
		EntityID destroyedEntityID = m_globalInfo.entities.begin()->first;
		m_globalInfo.entities.erase(destroyedEntityID);
		emit m_globalInfo.signalEmitter.destroyEntitySignal(destroyedEntityID);
	}
	m_globalInfo.currentEntityID = NO_ENTITY;
	m_globalInfo.otherSelectedEntityIDs.clear();
}

void OpenSceneCommand::redo() {
	m_globalInfo.entities = m_newEntities;
	for (const auto& newEntity : m_newEntities) {
		emit m_globalInfo.signalEmitter.createEntitySignal(newEntity.first);
	}
}