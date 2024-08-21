#include "file_menu.h"
#include "../common/scene_manager.h"
#include "new_scene_message_box.h"
#include <QKeySequence>
#include <QFileDialog>

FileMenu::FileMenu(GlobalInfo& globalInfo) : QMenu("&File"), m_globalInfo(globalInfo) {
	m_newSceneAction = addAction("New Scene", this, &FileMenu::newScene);
	m_newSceneAction->setShortcut(QKeySequence::fromString("Ctrl+N"));
	m_openSceneAction = addAction("Open Scene...", this, &FileMenu::openScene);
	m_openSceneAction->setShortcut(QKeySequence::fromString("Ctrl+O"));
	m_saveSceneAction = addAction("Save Scene", this, &FileMenu::saveScene);
	m_saveSceneAction->setShortcut(QKeySequence::fromString("Ctrl+S"));
	m_saveSceneAsAction = addAction("Save Scene as...", this, &FileMenu::saveSceneAs);
	m_saveSceneAsAction->setShortcut(QKeySequence::fromString("Shift+Ctrl+S"));
}

void FileMenu::newScene() {
	NewSceneMessageBox newSceneMessageBox(m_globalInfo);
}

void FileMenu::openScene() {
	QFileDialog fileDialog = QFileDialog();
	fileDialog.setWindowTitle("NutshellEngine - Open Scene...");
	fileDialog.setWindowIcon(QIcon("assets/icon.png"));
	fileDialog.setNameFilter("NutshellEngine Scene (*.ntsn)");
	if (std::filesystem::exists(m_globalInfo.projectDirectory + "/assets/")) {
		fileDialog.setDirectory(QString::fromStdString(m_globalInfo.projectDirectory + "/assets/"));
	}
	else if (m_globalInfo.projectDirectory != "") {
		fileDialog.setDirectory(QString::fromStdString(m_globalInfo.projectDirectory));
	}

	if (fileDialog.exec()) {
		std::string filePath = fileDialog.selectedFiles()[0].toStdString();
		SceneManager::openScene(m_globalInfo, filePath);
	}
}

void FileMenu::saveScene() {
	if (m_globalInfo.currentScenePath == "") {
		saveSceneAs();
	}
	else {
		SceneManager::saveScene(m_globalInfo, m_globalInfo.currentScenePath);
	}
}

void FileMenu::saveSceneAs() {
	QFileDialog fileDialog = QFileDialog();
	fileDialog.setWindowTitle("NutshellEngine - Save Scene as...");
	fileDialog.setDefaultSuffix("ntsn");
	if (std::filesystem::exists(m_globalInfo.projectDirectory + "/assets/")) {
		fileDialog.setDirectory(QString::fromStdString(m_globalInfo.projectDirectory + "/assets/"));
	}
	else if (m_globalInfo.projectDirectory != "") {
		fileDialog.setDirectory(QString::fromStdString(m_globalInfo.projectDirectory));
	}

	if (fileDialog.exec()) {
		std::string filePath = fileDialog.selectedFiles()[0].toStdString();
		SceneManager::saveScene(m_globalInfo, filePath);
	}
}
