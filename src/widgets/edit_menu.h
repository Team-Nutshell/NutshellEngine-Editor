#pragma once
#include "../common/common.h"
#include <QMenu>
#include <QAction>

class EditMenu : public QMenu {
	Q_OBJECT
public:
	EditMenu(GlobalInfo& globalInfo);

private:
	void copyEntity();
	void pasteEntity();

private slots:
	void onSelectEntity();

private:
	GlobalInfo& m_globalInfo;

	QAction* m_undoAction;
	QAction* m_redoAction;
	QAction* m_copyEntityAction;
	QAction* m_pasteEntityAction;
};