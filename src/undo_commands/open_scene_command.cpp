#include "open_scene_command.h"
#include "select_asset_entities_command.h"
#include "../common/save_title_changer.h"
#include "../widgets/main_window.h"

OpenSceneCommand::OpenSceneCommand(GlobalInfo& globalInfo, const std::vector<Entity>& newEntities, const std::string& newScenePath) : m_globalInfo(globalInfo), m_newEntities(newEntities), m_previousScenePath(globalInfo.currentScenePath), m_newScenePath(newScenePath), m_previousSceneModified(globalInfo.mainWindow->windowTitle()[0] == '*') {
	setText(QString::fromStdString(m_globalInfo.localization.getString("undo_open_scene", { m_newScenePath })));

	m_previousEntities.resize(globalInfo.entities.size());
	for (int i = 0; i < m_globalInfo.mainWindow->entityPanel->entityList->count(); i++) {
		EntityListItem* entityListItem = static_cast<EntityListItem*>(globalInfo.mainWindow->entityPanel->entityList->item(i));

		m_previousEntities[i] = globalInfo.entities[entityListItem->entityID];
	}
}

void OpenSceneCommand::undo() {
	m_globalInfo.selectionUndoStack->push(new SelectAssetEntitiesCommand(m_globalInfo, SelectionType::Entities, "", NO_ENTITY, {}));
	while (!m_globalInfo.entities.empty()) {
		EntityID destroyedEntityID = m_globalInfo.entities.begin()->first;
		m_globalInfo.entities.erase(destroyedEntityID);
		emit m_globalInfo.signalEmitter.destroyEntitySignal(destroyedEntityID);
	}

	for (const Entity& previousEntity : m_previousEntities) {
		m_globalInfo.entities[previousEntity.entityID] = previousEntity;
		emit m_globalInfo.signalEmitter.createEntitySignal(previousEntity.entityID);
	}
	m_globalInfo.currentScenePath = m_previousScenePath;
	m_globalInfo.mainWindow->updateTitle();

	if (m_previousSceneModified) {
		SaveTitleChanger::change(m_globalInfo.mainWindow);
	}
	else {
		SaveTitleChanger::reset(m_globalInfo.mainWindow);
	}
}

void OpenSceneCommand::redo() {
	m_globalInfo.selectionUndoStack->push(new SelectAssetEntitiesCommand(m_globalInfo, SelectionType::Entities, "", NO_ENTITY, {}));
	while (!m_globalInfo.entities.empty()) {
		EntityID destroyedEntityID = m_globalInfo.entities.begin()->first;
		m_globalInfo.entities.erase(destroyedEntityID);
		emit m_globalInfo.signalEmitter.destroyEntitySignal(destroyedEntityID);
	}

	for (const Entity& newEntity : m_newEntities) {
		m_globalInfo.entities[newEntity.entityID] = newEntity;
		emit m_globalInfo.signalEmitter.createEntitySignal(newEntity.entityID);
	}
	m_globalInfo.currentScenePath = m_newScenePath;
	m_globalInfo.mainWindow->updateTitle();

	SaveTitleChanger::reset(m_globalInfo.mainWindow);
}