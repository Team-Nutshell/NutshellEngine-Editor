#include "clear_scene_command.h"
#include "../common/save_title_changer.h"
#include "../widgets/main_window.h"

ClearSceneCommand::ClearSceneCommand(GlobalInfo& globalInfo, const std::string& previousScenePath) : m_globalInfo(globalInfo), m_previousScenePath(previousScenePath) {
	setText("Clear Scene");
	m_previousEntities = globalInfo.entities;
}

void ClearSceneCommand::undo() {
	m_globalInfo.entities = m_previousEntities;
	for (const auto& previousEntity : m_previousEntities) {
		emit m_globalInfo.signalEmitter.createEntitySignal(previousEntity.first);
	}
	m_globalInfo.currentScenePath = m_previousScenePath;
	m_globalInfo.mainWindow->updateTitle();
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