#pragma once
#include "../common/global_info.h"
#include <QMenu>
#include <QAction>

class ProjectMenu : public QMenu {
	Q_OBJECT
public:
	ProjectMenu(GlobalInfo& globalInfo);

private:
	void launchBuild();
	void openProjectSettings();
	void importGlobalResources();
	void updateBaseProject();

private:
	GlobalInfo& m_globalInfo;

	QAction* m_buildAction;
	QAction* m_openProjectSettingsAction;
	QAction* m_importGlobalResources;
	QAction* m_updateBaseProject;
};