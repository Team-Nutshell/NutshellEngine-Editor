#pragma once
#include "../common/common.h"
#include "open_project_widget.h"
#include "new_project_widget.h"
#include <QWidget>
#include <QLabel>
#include <memory>

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

	std::unique_ptr<QLabel> m_logoLabel;
	std::unique_ptr<QPixmap> m_logoPixmap;
	std::unique_ptr<OpenProjectWidget> m_openProjectWidget;
	std::unique_ptr<NewProjectWidget> m_newProjectWidget;
};