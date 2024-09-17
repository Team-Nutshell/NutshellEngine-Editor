#include "create_entity_command.h"

CreateEntityCommand::CreateEntityCommand(GlobalInfo& globalInfo, const std::string& name) : m_globalInfo(globalInfo) {
	m_entityID = NO_ENTITY;
	uint32_t entityNameIndex = 0;
	if (m_globalInfo.findEntityByName(name) == NO_ENTITY) {
		m_entityName = name;
	}
	else {
		while (m_globalInfo.findEntityByName(name + "_" + std::to_string(entityNameIndex)) != NO_ENTITY) {
			entityNameIndex++;
		}
		m_entityName = name + "_" + std::to_string(entityNameIndex);
	}
	setText(QString::fromStdString(m_globalInfo.localization.getString("undo_create_entity", { m_entityName })));
}

void CreateEntityCommand::undo() {
	m_globalInfo.entities.erase(m_entityID);
	emit m_globalInfo.signalEmitter.destroyEntitySignal(m_entityID);
}

void CreateEntityCommand::redo() {
	Entity newEntity;
	if (m_entityID == NO_ENTITY) {
		newEntity.entityID = m_globalInfo.globalEntityID++;
		m_entityID = newEntity.entityID;
	}
	else {
		newEntity.entityID = m_entityID;
	}
	newEntity.name = m_entityName;
	m_globalInfo.entities[m_entityID] = newEntity;
	emit m_globalInfo.signalEmitter.createEntitySignal(m_entityID);
}