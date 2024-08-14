#include "copy_entities_command.h"
#include "../widgets/main_window.h"
#include "../widgets/entity_list.h"

CopyEntitiesCommand::CopyEntitiesCommand(GlobalInfo& globalInfo, std::vector<Entity> entities) : m_globalInfo(globalInfo) {
	m_copiedEntities = entities;
	m_pastedEntityIDs.resize(m_copiedEntities.size());
	for (size_t i = 0; i < m_copiedEntities.size(); i++) {
		Entity& copiedEntity = m_copiedEntities[i];

		uint32_t entityNameIndex = 0;
		if (m_globalInfo.findEntityByName(copiedEntity.name) == NO_ENTITY) {
			m_pastedEntityNames.push_back(copiedEntity.name);
		}
		else {
			while (m_globalInfo.findEntityByName(copiedEntity.name + "_" + std::to_string(entityNameIndex)) != NO_ENTITY) {
				entityNameIndex++;
			}
			m_pastedEntityNames.push_back(copiedEntity.name + "_" + std::to_string(entityNameIndex));
		}
	}
	if (m_copiedEntities.size() == 1) {
		setText("Copy Entity " + QString::fromStdString(m_copiedEntities[0].name) + " to Entity " + QString::fromStdString(m_pastedEntityNames[0]));
	}
	else {
		setText("Copy multiple Entities.");
	}
}

void CopyEntitiesCommand::undo() {
	for (EntityID pastedEntityID : m_pastedEntityIDs) {
		m_globalInfo.entities.erase(pastedEntityID);
		emit m_globalInfo.signalEmitter.destroyEntitySignal(pastedEntityID);
		m_globalInfo.otherSelectedEntityIDs.erase(pastedEntityID);
		if (pastedEntityID == m_globalInfo.currentEntityID) {
			m_globalInfo.currentEntityID = NO_ENTITY;
		}
	}
}

void CopyEntitiesCommand::redo() {
	m_globalInfo.otherSelectedEntityIDs.clear();
	for (size_t i = 0; i < m_copiedEntities.size(); i++) {
		Entity& copiedEntity = m_copiedEntities[i];

		Entity pastedEntity = copiedEntity;
		pastedEntity.entityID = m_globalInfo.globalEntityID++;
		pastedEntity.name = m_pastedEntityNames[i];
		m_globalInfo.entities[pastedEntity.entityID] = pastedEntity;
		m_pastedEntityIDs[i] = pastedEntity.entityID;
		emit m_globalInfo.signalEmitter.createEntitySignal(m_pastedEntityIDs[i]);

		if (i == 0) {
			m_globalInfo.currentEntityID = pastedEntity.entityID;
			emit m_globalInfo.signalEmitter.selectEntitySignal();
		}
		else {
			m_globalInfo.otherSelectedEntityIDs.insert(pastedEntity.entityID);
		}
	}
	m_globalInfo.mainWindow->entityPanel->entityList->updateSelection();
}