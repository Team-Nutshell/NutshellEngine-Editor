#pragma once
#include "../common/common.h"
#include <QMenu>
#include <QAction>
#include <memory>

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