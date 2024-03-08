#include "new_scene_message_box.h"
#include "../common/scene_manager.h"

NewSceneMessageBox::NewSceneMessageBox(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setWindowTitle("New Scene");
	setText("Do you want to create a new scene?\nAll unsaved changes will be lost.");
	setStandardButtons(QMessageBox::StandardButton::Ok | QMessageBox::StandardButton::Cancel);
	int ret = exec();
	if (ret == QMessageBox::StandardButton::Ok) {
		SceneManager::newScene(m_globalInfo);
	}
}