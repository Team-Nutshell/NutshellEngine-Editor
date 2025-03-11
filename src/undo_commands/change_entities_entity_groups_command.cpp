#include "change_entities_entity_groups_command.h"

ChangeEntitiesEntityGroupsCommand::ChangeEntitiesEntityGroupsCommand(GlobalInfo& globalInfo, const std::vector<EntityID>& entityIDs, const std::vector<std::string>& entityGroups) : m_globalInfo(globalInfo) {
	if (entityIDs.size() == 1) {
		setText(QString::fromStdString(m_globalInfo.localization.getString("undo_change_entity_groups_entity", { m_globalInfo.entities[entityIDs[0]].name })));
	}
	else {
		setText(QString::fromStdString(m_globalInfo.localization.getString("undo_change_entity_groups_entities")));
	}
	m_entityIDs = entityIDs;
	for (size_t i = 0; i < m_entityIDs.size(); i++) {
		m_oldEntityGroups.push_back(m_globalInfo.entities[m_entityIDs[i]].entityGroups);
	}
	m_newEntityGroups = entityGroups;
}

void ChangeEntitiesEntityGroupsCommand::undo() {
	for (size_t i = 0; i < m_entityIDs.size(); i++) {
		m_globalInfo.entities[m_entityIDs[i]].entityGroups = m_oldEntityGroups[i];
	}
}

void ChangeEntitiesEntityGroupsCommand::redo() {
	for (size_t i = 0; i < m_entityIDs.size(); i++) {
		m_globalInfo.entities[m_entityIDs[i]].entityGroups = m_newEntityGroups;
	}
}
