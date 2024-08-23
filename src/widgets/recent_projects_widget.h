#pragma once
#include "../common/global_info.h"
#include <QWidget>
#include <string>

class RecentProjectsWidget : public QWidget {
	Q_OBJECT
public:
	RecentProjectsWidget(GlobalInfo& globalInfo);

signals:
	void openProject(const std::string&);

private slots:
	void onProjectOpened(const std::string& projectDirectory);
	void onRemoveRecentProjectButtonClicked();

private:
	GlobalInfo& m_globalInfo;

public:
	QWidget* projectsWidget;
};