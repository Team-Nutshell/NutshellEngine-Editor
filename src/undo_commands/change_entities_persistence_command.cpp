#include "change_entities_persistence_command.h"

ChangeEntitiesPersistenceCommand::ChangeEntitiesPersistenceCommand(GlobalInfo& globalInfo, const std::vector<EntityID>& entityIDs, bool isPersistent) : m_globalInfo(globalInfo) {
	if (entityIDs.size() == 1) {
		setText(QString::fromStdString(m_globalInfo.localization.getString("undo_change_persistence_entity", { m_globalInfo.entities[entityIDs[0]].name })));
	}
	else {
		setText(QString::fromStdString(m_globalInfo.localization.getString("undo_change_persistence_entities")));
	}
	m_entityIDs = entityIDs;
	for (size_t i = 0; i < m_entityIDs.size(); i++) {
		m_previousEntitiesPersistence.push_back(m_globalInfo.entities[entityIDs[i]].isPersistent);
	}
	m_newEntitiesPersistence = isPersistent;
}

void ChangeEntitiesPersistenceCommand::undo() {
	for (size_t i = 0; i < m_entityIDs.size(); i++) {
		m_globalInfo.entities[m_entityIDs[i]].isPersistent = m_previousEntitiesPersistence[i];
		emit m_globalInfo.signalEmitter.changeEntityPersistenceSignal(m_entityIDs[i], m_previousEntitiesPersistence[i]);
	}
}

void ChangeEntitiesPersistenceCommand::redo() {
	for (size_t i = 0; i < m_entityIDs.size(); i++) {
		m_globalInfo.entities[m_entityIDs[i]].isPersistent = m_newEntitiesPersistence;
		emit m_globalInfo.signalEmitter.changeEntityPersistenceSignal(m_entityIDs[i], m_newEntitiesPersistence);
	}
}