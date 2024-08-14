#pragma once
#include "../common/global_info.h"
#include <QMenu>
#include <QAction>

class EditMenu : public QMenu {
	Q_OBJECT
public:
	EditMenu(GlobalInfo& globalInfo);

private:
	void copyEntities();
	void pasteEntities();

private slots:
	void onEntitySelected();

private:
	GlobalInfo& m_globalInfo;

	QAction* m_undoAction;
	QAction* m_redoAction;
	QAction* m_copyEntitiesAction;
	QAction* m_pasteEntitiesAction;
};