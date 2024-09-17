#include "new_scene_message_box.h"
#include "../common/scene_manager.h"

NewSceneMessageBox::NewSceneMessageBox(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setWindowTitle("NutshellEngine - " + QString::fromStdString(m_globalInfo.localization.getString("scene_new_scene")));
	setWindowIcon(QIcon("assets/icon.png"));

	setText(QString::fromStdString(m_globalInfo.localization.getString("scene_new_scene_confirm")));
	setStandardButtons(QMessageBox::StandardButton::Ok | QMessageBox::StandardButton::Cancel);
	int ret = exec();
	if (ret == QMessageBox::StandardButton::Ok) {
		SceneManager::newScene(m_globalInfo);
	}
}