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
	void duplicateEntities();

private slots:
	void onEntitySelected();

private:
	GlobalInfo& m_globalInfo;

	QAction* m_undoActionAction;
	QAction* m_redoActionAction;
	QAction* m_undoSelectionAction;
	QAction* m_redoSelectionAction;
	QAction* m_copyEntitiesAction;
	QAction* m_pasteEntitiesAction;
	QAction* m_duplicateEntitiesAction;
};