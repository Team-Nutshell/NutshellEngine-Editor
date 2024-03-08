#include "copy_entity_command.h"

CopyEntityCommand::CopyEntityCommand(GlobalInfo& globalInfo, EntityID entityID) : m_globalInfo(globalInfo) {
	m_copiedEntity = m_globalInfo.entities[entityID];
	uint32_t entityNameIndex = 0;
	while (m_globalInfo.findEntityByName(m_copiedEntity.name + "_" + std::to_string(entityNameIndex)) != NO_ENTITY) {
		entityNameIndex++;
	}
	m_passedEntityName = m_copiedEntity.name + "_" + std::to_string(entityNameIndex);
	setText("Copy Entity " + QString::fromStdString(m_copiedEntity.name) + " to Entity " + QString::fromStdString(m_passedEntityName));
}

void CopyEntityCommand::undo() {
	m_globalInfo.entities.erase(m_pastedEntityID);
	emit m_globalInfo.signalEmitter.destroyEntitySignal(m_pastedEntityID);
}

void CopyEntityCommand::redo() {
	Entity pastedEntity = m_copiedEntity;
	pastedEntity.entityID = m_globalInfo.globalEntityID++;
	pastedEntity.name = m_passedEntityName;
	m_globalInfo.entities[pastedEntity.entityID] = pastedEntity;
	m_pastedEntityID = pastedEntity.entityID;
	emit m_globalInfo.signalEmitter.createEntitySignal(m_pastedEntityID);
	m_globalInfo.currentEntityID = m_pastedEntityID;
	emit m_globalInfo.signalEmitter.selectEntitySignal();
}