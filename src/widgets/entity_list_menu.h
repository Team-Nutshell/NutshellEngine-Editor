#pragma once
#include "../common/global_info.h"
#include <QMenu>
#include <QAction>

class EntityList;

class EntityListMenu : public QMenu {
	Q_OBJECT
public:
	EntityListMenu(GlobalInfo& globalInfo);

private slots:
	void renameEntities();
	void newEntities();
	void deleteEntities();
	void duplicateEntities();

private:
	GlobalInfo& m_globalInfo;

public:
	EntityList* entityList = nullptr;

	QAction* renameAction;
	QAction* newEntityAction;
	QAction* deleteAction;
	QAction* duplicateAction;
};