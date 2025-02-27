#pragma once
#include "../common/global_info.h"
#include <QMenu>
#include <QAction>
#include <string>

class ScriptList;

class ScriptListMenu : public QMenu {
	Q_OBJECT
public:
	ScriptListMenu(GlobalInfo& globalInfo);

private slots:
	void newScript();
	void renameScript();
	void deleteScript();
	void duplicateScript();
	void copyName();

private:
	GlobalInfo& m_globalInfo;

public:
	ScriptList* scriptList = nullptr;

	std::string scriptName = "";

	QAction* newAction;
	QAction* renameAction;
	QAction* deleteAction;
	QAction* duplicateAction;
	QAction* copyNameAction;
};