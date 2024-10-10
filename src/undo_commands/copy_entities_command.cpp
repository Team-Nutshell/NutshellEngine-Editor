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
		m_pastedEntityIDs[i] = m_globalInfo.globalEntityID++;
	}
	if (m_copiedEntities.size() == 1) {
		setText(QString::fromStdString(m_globalInfo.localization.getString("undo_copy_entities", { m_copiedEntities[0].name, m_pastedEntityNames[0] })));
	}
	else {
		setText(QString::fromStdString(m_globalInfo.localization.getString("undo_copy_entities")));
	}
}

void CopyEntitiesCommand::undo() {
	for (EntityID pastedEntityID : m_pastedEntityIDs) {
		m_globalInfo.entities.erase(pastedEntityID);
		emit m_globalInfo.signalEmitter.destroyEntitySignal(pastedEntityID);
		m_globalInfo.otherSelectedEntityIDs.erase(pastedEntityID);
		if (pastedEntityID == m_globalInfo.currentEntityID) {
			m_globalInfo.clearSelectedEntities();
		}
	}
}

void CopyEntitiesCommand::redo() {
	m_globalInfo.otherSelectedEntityIDs.clear();
	for (size_t i = 0; i < m_copiedEntities.size(); i++) {
		Entity& copiedEntity = m_copiedEntities[i];

		Entity pastedEntity = copiedEntity;
		pastedEntity.entityID = m_pastedEntityIDs[i];
		pastedEntity.name = m_pastedEntityNames[i];
		m_globalInfo.entities[pastedEntity.entityID] = pastedEntity;
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