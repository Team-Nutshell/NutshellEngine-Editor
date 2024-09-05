#include "clear_scene_command.h"
#include "../common/save_title_changer.h"
#include "../widgets/main_window.h"

ClearSceneCommand::ClearSceneCommand(GlobalInfo& globalInfo) : m_globalInfo(globalInfo), m_previousEntities(globalInfo.entities), m_previousScenePath(globalInfo.currentScenePath), m_previousSceneModified(globalInfo.mainWindow->windowTitle()[0] == '*') {
	setText("Clear Scene");
}

void ClearSceneCommand::undo() {
	m_globalInfo.entities = m_previousEntities;
	for (const auto& previousEntity : m_previousEntities) {
		emit m_globalInfo.signalEmitter.createEntitySignal(previousEntity.first);
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

void ClearSceneCommand::redo() {
	m_globalInfo.clearSelectedEntities();
	while (!m_globalInfo.entities.empty()) {
		EntityID destroyedEntityID = m_globalInfo.entities.begin()->first;
		m_globalInfo.entities.erase(destroyedEntityID);
		emit m_globalInfo.signalEmitter.destroyEntitySignal(destroyedEntityID);
	}
	m_globalInfo.currentScenePath = "";
	m_globalInfo.mainWindow->updateTitle();

	SaveTitleChanger::reset(m_globalInfo.mainWindow);
}