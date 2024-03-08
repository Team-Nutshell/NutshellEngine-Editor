#include "file_menu.h"
#include <QKeySequence>

FileMenu::FileMenu(GlobalInfo& globalInfo): QMenu("&File"), m_globalInfo(globalInfo) {
	m_newSceneAction = addAction("New Scene", this, &FileMenu::newScene);
	m_newSceneAction->setShortcut(QKeySequence::fromString("Ctrl+N"));
	m_openSceneAction = addAction("Open Scene...", this, &FileMenu::newScene);
	m_openSceneAction->setShortcut(QKeySequence::fromString("Ctrl+O"));
	m_saveSceneAction = addAction("Save Scene", this, &FileMenu::saveScene);
	m_saveSceneAction->setShortcut(QKeySequence::fromString("Ctrl+S"));
	m_saveSceneAsAction = addAction("Save Scene as...", this, &FileMenu::saveSceneAs);
	m_saveSceneAsAction->setShortcut(QKeySequence::fromString("Shift+Ctrl+S"));
}

void FileMenu::newScene() {

}

void FileMenu::openScene() {

}

void FileMenu::saveScene() {

}

void FileMenu::saveSceneAs() {

}
