#pragma once
#include "../common/global_info.h"
#include <QMenu>
#include <QAction>

class EntityListMenu : public QMenu {
	Q_OBJECT
public:
	EntityListMenu(GlobalInfo& globalInfo);

private slots:
	void newEntity();
	void deleteEntity();

private:
	GlobalInfo& m_globalInfo;

public:
	QAction* newEntityAction;
	QAction* deleteEntityAction;
};