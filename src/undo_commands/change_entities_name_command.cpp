#include "change_entities_name_command.h"
#include <algorithm>

ChangeEntitiesNameCommand::ChangeEntitiesNameCommand(GlobalInfo& globalInfo, std::vector<EntityID> entityIDs, const std::string& name) : m_globalInfo(globalInfo) {
	m_entityIDs = entityIDs;
	for (size_t i = 0; i < m_entityIDs.size(); i++) {
		m_previousEntityNames.push_back(m_globalInfo.entities[entityIDs[i]].name);
		uint32_t entityNameIndex = 0;
		if ((m_globalInfo.findEntityByName(name) == NO_ENTITY) &&
			(std::find(m_newEntityNames.begin(), m_newEntityNames.end(), name) == m_newEntityNames.end())) {
			m_newEntityNames.push_back(name);
		}
		else {
			while ((m_globalInfo.findEntityByName(name + "_" + std::to_string(entityNameIndex)) != NO_ENTITY) ||
				(std::find(m_newEntityNames.begin(), m_newEntityNames.end(), name + "_" + std::to_string(entityNameIndex)) != m_newEntityNames.end())) {
				entityNameIndex++;
			}
			m_newEntityNames.push_back(name + "_" + std::to_string(entityNameIndex));
		}
	}
	if (entityIDs.size() == 1) {
		setText(QString::fromStdString(m_globalInfo.localization.getString("undo_change_name_entity", { m_globalInfo.entities[entityIDs[0]].name, m_newEntityNames[0] })));
	}
	else {
		setText(QString::fromStdString(m_globalInfo.localization.getString("undo_change_name_entities")));
	}
}

void ChangeEntitiesNameCommand::undo() {
	for (size_t i = 0; i < m_entityIDs.size(); i++) {
		m_globalInfo.entities[m_entityIDs[i]].name = m_previousEntityNames[i];
		emit m_globalInfo.signalEmitter.changeEntityNameSignal(m_entityIDs[i], m_previousEntityNames[i]);
	}
}

void ChangeEntitiesNameCommand::redo() {
	for (size_t i = 0; i < m_entityIDs.size(); i++) {
		m_globalInfo.entities[m_entityIDs[i]].name = m_newEntityNames[i];
		emit m_globalInfo.signalEmitter.changeEntityNameSignal(m_entityIDs[i], m_newEntityNames[i]);
	}
}