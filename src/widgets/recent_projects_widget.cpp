#include "recent_projects_widget.h"
#include "recent_project_widget.h"
#include "../../external/nlohmann/json.hpp"
#include <QScrollArea>
#include <QVBoxLayout>
#include <fstream>
#include <filesystem>

RecentProjectsWidget::RecentProjectsWidget(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setLayout(new QVBoxLayout());
	layout()->setContentsMargins(0, 0, 0, 0);
	layout()->setAlignment(Qt::AlignmentFlag::AlignTop);
	projectsWidget = new QWidget();
	projectsWidget->setLayout(new QVBoxLayout());
	projectsWidget->layout()->setContentsMargins(0, 0, 0, 0);
	projectsWidget->layout()->setAlignment(Qt::AlignmentFlag::AlignTop);
	QScrollArea* scrollPrimitivesWidget = new QScrollArea();
	scrollPrimitivesWidget->setWidgetResizable(true);
	scrollPrimitivesWidget->setWidget(projectsWidget);
	layout()->addWidget(scrollPrimitivesWidget);

	std::fstream recentProjectsFile("recent_projects.json", std::ios::in);
	if (recentProjectsFile.is_open()) {
		if (!nlohmann::json::accept(recentProjectsFile)) {
			return;
		}
	}
	else {
		return;
	}

	recentProjectsFile = std::fstream("recent_projects.json", std::ios::in);
	nlohmann::json j = nlohmann::json::parse(recentProjectsFile);

	recentProjectsFile = std::fstream("recent_projects.json", std::ios::out | std::ios::trunc);

	if (j.contains("projects")) {
		for (size_t i = 0; i < j["projects"].size(); i++) {
			const auto& project = j["projects"][i];
			if (std::filesystem::exists(project)) {
				RecentProjectWidget* recentProjectWidget = new RecentProjectWidget(m_globalInfo, project);
				projectsWidget->layout()->addWidget(recentProjectWidget);

				connect(recentProjectWidget, &RecentProjectWidget::openProject, this, &RecentProjectsWidget::onProjectOpened);
				connect(recentProjectWidget, &RecentProjectWidget::removeRecentProject, this, &RecentProjectsWidget::onRemoveRecentProjectButtonClicked);
			}
			else {
				j["projects"].erase(i);
			}
		}
	}

	recentProjectsFile << j.dump(1, '\t');
}

void RecentProjectsWidget::onProjectOpened(const std::string& projectDirectory) {
	emit openProject(projectDirectory);
}

void RecentProjectsWidget::onRemoveRecentProjectButtonClicked() {
	QWidget* senderWidget = static_cast<QWidget*>(sender());

	nlohmann::json j;

	std::fstream recentProjectsFile("recent_projects.json", std::ios::in);
	if (recentProjectsFile.is_open()) {
		if (nlohmann::json::accept(recentProjectsFile)) {
			recentProjectsFile = std::fstream("recent_projects.json", std::ios::in);
			j = nlohmann::json::parse(recentProjectsFile);
		}
		else {
			return;
		}
	}
	else {
		return;
	}

	recentProjectsFile = std::fstream("recent_projects.json", std::ios::out | std::ios::trunc);
	if (j.contains("projects")) {
		for (size_t i = 0; i < j["projects"].size(); i++) {
			const auto& project = j["projects"][i];
			if (project == static_cast<RecentProjectWidget*>(senderWidget)->getProjectDirectory()) {
				j["projects"].erase(i);
			}
		}
	}

	recentProjectsFile << j.dump(1, '\t');

	projectsWidget->layout()->takeAt(projectsWidget->layout()->indexOf(senderWidget));
	delete senderWidget;
}