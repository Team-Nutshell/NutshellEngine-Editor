#include "destroy_entity_command.h"

DestroyEntityCommand::DestroyEntityCommand(GlobalInfo& globalInfo, EntityID entityID) : m_globalInfo(globalInfo) {
	setText("Destroy Entity " + QString::fromStdString(m_globalInfo.entities[entityID].name));
	m_destroyedEntity = m_globalInfo.entities[entityID];
}

void DestroyEntityCommand::undo() {
	m_globalInfo.entities[m_destroyedEntity.entityID] = m_destroyedEntity;
	emit m_globalInfo.signalEmitter.createEntitySignal(m_destroyedEntity.entityID);
}

void DestroyEntityCommand::redo() {
	m_globalInfo.entities.erase(m_destroyedEntity.entityID);
	emit m_globalInfo.signalEmitter.destroyEntitySignal(m_destroyedEntity.entityID);
}