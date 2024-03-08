#include "change_entity_name_command.h"

ChangeEntityNameCommand::ChangeEntityNameCommand(GlobalInfo& globalInfo, EntityID entityID, const std::string& name) : m_globalInfo(globalInfo) {
	setText("Change Entity " + QString::fromStdString(m_globalInfo.entities[entityID].name) + " Name to " + QString::fromStdString(name));
	m_entityID = entityID;
	m_previousEntityName = m_globalInfo.entities[entityID].name;
	m_newEntityName = name;
}

void ChangeEntityNameCommand::undo() {
	m_globalInfo.entities[m_entityID].name = m_previousEntityName;
	emit m_globalInfo.signalEmitter.changeEntityNameSignal(m_entityID, m_previousEntityName);
}

void ChangeEntityNameCommand::redo() {
	m_globalInfo.entities[m_entityID].name = m_newEntityName;
	emit m_globalInfo.signalEmitter.changeEntityNameSignal(m_entityID, m_newEntityName);
}