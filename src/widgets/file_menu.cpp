#include "file_menu.h"
#include "../common/scene_manager.h"
#include "new_scene_message_box.h"
#include "main_window.h"
#include <QKeySequence>
#include <QFileDialog>

FileMenu::FileMenu(GlobalInfo& globalInfo) : QMenu("&" + QString::fromStdString(globalInfo.localization.getString("header_file"))), m_globalInfo(globalInfo) {
	m_newSceneAction = addAction(QString::fromStdString(m_globalInfo.localization.getString("header_file_new_scene")), this, &FileMenu::newScene);
	m_newSceneAction->setShortcut(QKeySequence::fromString("Ctrl+N"));
	m_openSceneAction = addAction(QString::fromStdString(m_globalInfo.localization.getString("header_file_open_scene")), this, &FileMenu::openScene);
	m_openSceneAction->setShortcut(QKeySequence::fromString("Ctrl+O"));
	m_saveSceneAction = addAction(QString::fromStdString(m_globalInfo.localization.getString("header_file_save_scene")), this, &FileMenu::saveScene);
	m_saveSceneAction->setShortcut(QKeySequence::fromString("Ctrl+S"));
	m_saveSceneAsAction = addAction(QString::fromStdString(m_globalInfo.localization.getString("header_file_save_scene_as")), this, &FileMenu::saveSceneAs);
	m_saveSceneAsAction->setShortcut(QKeySequence::fromString("Shift+Ctrl+S"));
}

void FileMenu::newScene() {
	if (m_globalInfo.mainWindow->windowTitle()[0] == '*') {
		NewSceneMessageBox newSceneMessageBox(m_globalInfo);
	}
	else {
		SceneManager::newScene(m_globalInfo);
	}
}

void FileMenu::openScene() {
	QFileDialog fileDialog = QFileDialog();
	fileDialog.setWindowTitle("NutshellEngine - " + QString::fromStdString(m_globalInfo.localization.getString("header_file_open_scene")));
	fileDialog.setWindowIcon(QIcon("assets/icon.png"));
	fileDialog.setNameFilter("NutshellEngine Scene (*.ntsn)");
	if (std::filesystem::exists(m_globalInfo.projectDirectory + "/assets/")) {
		fileDialog.setDirectory(QString::fromStdString(m_globalInfo.projectDirectory + "/assets/"));
	}
	else if (!m_globalInfo.projectDirectory.empty()) {
		fileDialog.setDirectory(QString::fromStdString(m_globalInfo.projectDirectory));
	}

	if (fileDialog.exec()) {
		std::string filePath = fileDialog.selectedFiles()[0].toStdString();
		SceneManager::openScene(m_globalInfo, filePath);
	}
}

void FileMenu::saveScene() {
	if (m_globalInfo.currentScenePath.empty()) {
		saveSceneAs();
	}
	else {
		SceneManager::saveScene(m_globalInfo, m_globalInfo.currentScenePath);
	}
}

void FileMenu::saveSceneAs() {
	QFileDialog fileDialog = QFileDialog();
	fileDialog.setWindowTitle("NutshellEngine - " + QString::fromStdString(m_globalInfo.localization.getString("header_file_save_scene_as")));
	fileDialog.setDefaultSuffix("ntsn");
	if (std::filesystem::exists(m_globalInfo.projectDirectory + "/assets/")) {
		fileDialog.setDirectory(QString::fromStdString(m_globalInfo.projectDirectory + "/assets/"));
	}
	else if (!m_globalInfo.projectDirectory.empty()) {
		fileDialog.setDirectory(QString::fromStdString(m_globalInfo.projectDirectory));
	}

	if (fileDialog.exec()) {
		std::string filePath = fileDialog.selectedFiles()[0].toStdString();
		SceneManager::saveScene(m_globalInfo, filePath);
	}
}
