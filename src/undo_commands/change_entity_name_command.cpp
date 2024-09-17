#include "change_entity_name_command.h"

ChangeEntityNameCommand::ChangeEntityNameCommand(GlobalInfo& globalInfo, EntityID entityID, const std::string& name) : m_globalInfo(globalInfo) {
	m_entityID = entityID;
	m_previousEntityName = m_globalInfo.entities[entityID].name;
	uint32_t entityNameIndex = 0;
	if (m_globalInfo.findEntityByName(name) == NO_ENTITY) {
		m_newEntityName = name;
	}
	else {
		while (m_globalInfo.findEntityByName(name + "_" + std::to_string(entityNameIndex)) != NO_ENTITY) {
			entityNameIndex++;
		}
		m_newEntityName = name + "_" + std::to_string(entityNameIndex);
	}
	setText(QString::fromStdString(m_globalInfo.localization.getString("undo_change_entity_name", { m_globalInfo.entities[entityID].name, m_newEntityName })));
}

void ChangeEntityNameCommand::undo() {
	m_globalInfo.entities[m_entityID].name = m_previousEntityName;
	emit m_globalInfo.signalEmitter.changeEntityNameSignal(m_entityID, m_previousEntityName);
}

void ChangeEntityNameCommand::redo() {
	m_globalInfo.entities[m_entityID].name = m_newEntityName;
	emit m_globalInfo.signalEmitter.changeEntityNameSignal(m_entityID, m_newEntityName);
}