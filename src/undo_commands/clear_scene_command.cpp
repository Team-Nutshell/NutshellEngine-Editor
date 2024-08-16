#include "clear_scene_command.h"

ClearSceneCommand::ClearSceneCommand(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setText("Clear Scene");
	m_previousEntities = globalInfo.entities;
}

void ClearSceneCommand::undo() {
	m_globalInfo.entities = m_previousEntities;
	for (const auto& previousEntity : m_previousEntities) {
		emit m_globalInfo.signalEmitter.createEntitySignal(previousEntity.first);
	}
}

void ClearSceneCommand::redo() {
	m_globalInfo.currentEntityID = NO_ENTITY;
	m_globalInfo.otherSelectedEntityIDs.clear();
	while (!m_globalInfo.entities.empty()) {
		EntityID destroyedEntityID = m_globalInfo.entities.begin()->first;
		m_globalInfo.entities.erase(destroyedEntityID);
		emit m_globalInfo.signalEmitter.destroyEntitySignal(destroyedEntityID);
	}
}