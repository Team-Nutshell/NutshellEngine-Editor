#pragma once
#include "../common/global_info.h"
#include "open_project_widget.h"
#include "new_project_widget.h"
#include "recent_projects_widget.h"
#include <QWidget>
#include <QLabel>

class ProjectWindow : public QWidget {
	Q_OBJECT
public:
	ProjectWindow(GlobalInfo& globalInfo);

private slots:
	void onProjectDirectorySelected(const std::string& projectDirectory);
	void onNewProjectButtonClicked(const std::string& projectDirectory, const std::string& projectName);

private:
	void openMainWindow(const std::string& projectDirectory, const std::string& projectName);

private:
	GlobalInfo& m_globalInfo;

	QWidget* m_leftWidget;
	QWidget* m_rightWidget;

	QLabel* m_logoLabel;
	QPixmap* m_logoPixmap;
	OpenProjectWidget* m_openProjectWidget;
	NewProjectWidget* m_newProjectWidget;

	RecentProjectsWidget* m_recentProjectWidget;
};