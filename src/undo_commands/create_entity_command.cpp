#include "create_entity_command.h"

CreateEntityCommand::CreateEntityCommand(GlobalInfo& globalInfo, const std::string& name) : m_globalInfo(globalInfo) {
	setText("Create Entity " + QString::fromStdString(name));
	m_entityID = NO_ENTITY;
	m_entityName = name;
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