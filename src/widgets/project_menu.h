#pragma once
#include "../common/global_info.h"
#include <QMenu>
#include <QAction>

class ProjectMenu : public QMenu {
	Q_OBJECT
public:
	ProjectMenu(GlobalInfo& globalInfo);

private:
	void openProjectSettings();

private:
	GlobalInfo& m_globalInfo;

	QAction* m_openProjectSettingsAction;
};