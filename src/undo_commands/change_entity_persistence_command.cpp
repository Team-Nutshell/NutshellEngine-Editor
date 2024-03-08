#include "change_entity_persistence_command.h"

ChangeEntityPersistenceCommand::ChangeEntityPersistenceCommand(GlobalInfo& globalInfo, EntityID entityID, bool isPersistent) : m_globalInfo(globalInfo) {
	setText("Change Entity " + QString::fromStdString(m_globalInfo.entities[entityID].name) + " Persistence to " + (isPersistent ? "True" : "False"));
	m_entityID = entityID;
	m_previousEntityPersistence = m_globalInfo.entities[entityID].isPersistent;
	m_newEntityPersistence = isPersistent;
}

void ChangeEntityPersistenceCommand::undo() {
	m_globalInfo.entities[m_entityID].isPersistent = m_previousEntityPersistence;
	emit m_globalInfo.signalEmitter.changeEntityPersistenceSignal(m_entityID, m_previousEntityPersistence);
}

void ChangeEntityPersistenceCommand::redo() {
	m_globalInfo.entities[m_entityID].isPersistent = m_newEntityPersistence;
	emit m_globalInfo.signalEmitter.changeEntityPersistenceSignal(m_entityID, m_newEntityPersistence);
}