#pragma once
#include "../common/global_info.h"
#include "script_list_menu.h"
#include <QListWidget>
#include <QFileSystemWatcher>
#include <QKeyEvent>
#include <QWheelEvent>
#include <string>

class ScriptList : public QListWidget {
	Q_OBJECT
public:
	ScriptList(GlobalInfo& globalInfo);

	void newScript();
	bool renameScriptFile(const std::string& oldScriptName, const std::string& newScriptName);
	void renameScriptClass(const std::string& oldScriptName, const std::string& newScriptName);
	void deleteScript(const std::string& scriptName);
	void duplicateScript(const std::string& scriptName);

private:
	void updateScriptList();

	void openCodeEditor(const std::string& scriptName);

private slots:
	void onItemDoubleClicked(QListWidgetItem* listWidgetItem);
	void onDirectoryChanged();

	void showMenu(const QPoint& pos);

	void keyPressEvent(QKeyEvent* event);
	void wheelEvent(QWheelEvent* event);
	void onLineEditClose(QWidget* lineEdit, QAbstractItemDelegate::EndEditHint hint);

private:
	GlobalInfo& m_globalInfo;

	std::string m_scriptsDirectory;
	QFileSystemWatcher m_directoryWatcher;

public:
	ScriptListMenu* menu;

	std::string currentlyEditedItemName = "";
};