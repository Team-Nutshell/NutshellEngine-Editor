#pragma once
#include "../common/global_info.h"
#include "about_widget.h"
#include <QMenu>
#include <QAction>

class HelpMenu : public QMenu {
	Q_OBJECT
public:
	HelpMenu(GlobalInfo& globalInfo);

private:
	void openDocumentation();
	void aboutNutshellEngine();

private slots:
	void onAboutWidgetClose();

private:
	GlobalInfo& m_globalInfo;

	QAction* m_openDocumentationAction;
	QAction* m_aboutNutshellEngineAction;

	AboutWidget* m_aboutWidget = nullptr;
};