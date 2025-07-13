#include "destroy_entities_command.h"
#include "select_asset_entities_command.h"
#include "../widgets/main_window.h"
#include "../widgets/entity_list.h"

DestroyEntitiesCommand::DestroyEntitiesCommand(GlobalInfo& globalInfo, const std::vector<EntityID>& entityIDs) : m_globalInfo(globalInfo) {
	if (entityIDs.size() == 1) {
		setText(QString::fromStdString(m_globalInfo.localization.getString("undo_destroy_entity", { m_globalInfo.entities[entityIDs[0]].name })));
	}
	else {
		setText(QString::fromStdString(m_globalInfo.localization.getString("undo_destroy_entities")));
	}
	EntityList* entityList = m_globalInfo.mainWindow->entityPanel->entityList;
	for (EntityID entityID : entityIDs) {
		m_destroyedEntities.push_back({ m_globalInfo.entities[entityID], entityList->row(entityList->findItemWithEntityID(entityID)) });
	}
	std::sort(m_destroyedEntities.begin(), m_destroyedEntities.end(), [](const std::pair<Entity, int>& a, const std::pair<Entity, int>& b) {
		return a.second < b.second;
		});
}

void DestroyEntitiesCommand::undo() {
	EntityID currentEntityID = m_globalInfo.currentEntityID;
	std::set<EntityID> otherSelectedEntityIDs;
	EntityList* entityList = m_globalInfo.mainWindow->entityPanel->entityList;
	for (const auto& destroyEntity : m_destroyedEntities) {
		m_globalInfo.entities[destroyEntity.first.entityID] = destroyEntity.first;
		emit m_globalInfo.signalEmitter.createEntitySignal(destroyEntity.first.entityID);
		EntityListItem* entityListItem = static_cast<EntityListItem*>(entityList->takeItem(entityList->count() - 1));
		entityList->insertItem(destroyEntity.second, entityListItem);
		otherSelectedEntityIDs.insert(destroyEntity.first.entityID);
	}
	otherSelectedEntityIDs.erase(m_destroyedEntities.back().first.entityID);
	currentEntityID = m_destroyedEntities.back().first.entityID;
	m_globalInfo.selectionUndoStack->push(new SelectAssetEntitiesCommand(m_globalInfo, SelectionType::Entities, "", currentEntityID, otherSelectedEntityIDs));
}

void DestroyEntitiesCommand::redo() {
	EntityID currentEntityID = m_globalInfo.currentEntityID;
	std::set<EntityID> otherSelectedEntityIDs = m_globalInfo.otherSelectedEntityIDs;
	bool clearAllSelection = false;
	for (const auto& destroyEntity : m_destroyedEntities) {
		m_globalInfo.entities.erase(destroyEntity.first.entityID);
		emit m_globalInfo.signalEmitter.destroyEntitySignal(destroyEntity.first.entityID);
		m_globalInfo.otherSelectedEntityIDs.erase(destroyEntity.first.entityID);
		if (destroyEntity.first.entityID == m_globalInfo.currentEntityID) {
			clearAllSelection = true;
		}
	}

	if (clearAllSelection) {
		m_globalInfo.selectionUndoStack->push(new SelectAssetEntitiesCommand(m_globalInfo, SelectionType::Entities, "", NO_ENTITY, {}));
	}
	else {
		m_globalInfo.selectionUndoStack->push(new SelectAssetEntitiesCommand(m_globalInfo, SelectionType::Entities, "", currentEntityID, otherSelectedEntityIDs));
	}
}