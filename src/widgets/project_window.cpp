#include "project_window.h"
#include "separator_line.h"
#include "main_window.h"
#include "../../external/nlohmann/json.hpp"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QIcon>
#include <QPixmap>
#include <fstream>
#include <algorithm>

ProjectWindow::ProjectWindow(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setFixedSize(900, 400);
	setWindowTitle("NutshellEngine");
	setWindowIcon(QIcon("assets/icon.png"));

	setLayout(new QHBoxLayout());
	m_leftWidget = new QWidget();
	m_leftWidget->setLayout(new QVBoxLayout());
	m_leftWidget->layout()->setAlignment(Qt::AlignmentFlag::AlignCenter);
	m_leftWidget->layout()->setContentsMargins(0, 0, 0, 0);
	m_leftWidget->setFixedWidth(550);
	m_logoLabel = new QLabel();
	m_logoPixmap = new QPixmap("assets/logo.png");
	m_logoLabel->setPixmap(m_logoPixmap->scaledToWidth(m_leftWidget->width() - 20, Qt::TransformationMode::SmoothTransformation));
	m_leftWidget->layout()->addWidget(m_logoLabel);
	m_leftWidget->layout()->addWidget(new SeparatorLine());
	m_leftWidget->layout()->addWidget(new QLabel("<b>" + QString::fromStdString(m_globalInfo.localization.getString("open_project")) + "</b>"));
	m_openProjectWidget = new OpenProjectWidget(m_globalInfo);
	m_leftWidget->layout()->addWidget(m_openProjectWidget);
	m_leftWidget->layout()->addWidget(new SeparatorLine());
	m_leftWidget->layout()->addWidget(new QLabel("<b>" + QString::fromStdString(m_globalInfo.localization.getString("new_project")) + "</b>"));
	m_newProjectWidget = new NewProjectWidget(m_globalInfo);
	m_leftWidget->layout()->addWidget(m_newProjectWidget);
	layout()->addWidget(m_leftWidget);
	m_rightWidget = new QWidget();
	m_rightWidget->setLayout(new QVBoxLayout());
	m_rightWidget->setFixedWidth(300);
	m_rightWidget->layout()->setAlignment(Qt::AlignmentFlag::AlignTop);
	m_rightWidget->layout()->setContentsMargins(0, 0, 0, 0);
	m_rightWidget->layout()->addWidget(new QLabel("<b>" + QString::fromStdString(m_globalInfo.localization.getString("recent_projects")) + "</b>"));
	m_recentProjectWidget = new RecentProjectsWidget(m_globalInfo);
	m_rightWidget->layout()->addWidget(m_recentProjectWidget);
	layout()->addWidget(m_rightWidget);

	connect(m_openProjectWidget, &OpenProjectWidget::projectDirectorySelected, this, &ProjectWindow::onProjectDirectorySelected);
	connect(m_newProjectWidget, &NewProjectWidget::newProjectButtonClicked, this, &ProjectWindow::onNewProjectButtonClicked);
	connect(m_recentProjectWidget, &RecentProjectsWidget::openProject, this, &ProjectWindow::onProjectDirectorySelected);
}

void ProjectWindow::onNewProjectButtonClicked(const std::string& projectDirectory, const std::string& projectName) {
	std::filesystem::create_directory(projectDirectory);
	std::filesystem::copy("assets/base_project", projectDirectory, std::filesystem::copy_options::recursive);

	nlohmann::json jOptions;
	jOptions["windowTitle"] = projectName;
	jOptions["maxFPS"] = 60;
	std::filesystem::create_directory(projectDirectory + "/assets/options/");
	std::fstream optionsFile(projectDirectory + "/assets/options/options.ntop", std::ios::out | std::ios::trunc);
	optionsFile << jOptions.dump(1, '\t');
	optionsFile.close();

	nlohmann::json jProject;
	jProject["projectName"] = projectName;
	jProject["engineVersion"] = m_globalInfo.version;
	std::fstream projectFile(projectDirectory + "/project.ntpj", std::ios::out | std::ios::trunc);
	projectFile << jProject.dump(1, '\t');
	projectFile.close();

	openMainWindow(projectDirectory, projectName);
}

void ProjectWindow::onProjectDirectorySelected(const std::string& projectDirectory) {
	std::string projectName = projectDirectory.substr(projectDirectory.rfind('/') + 1);
	std::fstream projectFile(projectDirectory + "/project.ntpj", std::ios::in);
	if (projectFile.is_open()) {
		nlohmann::json j = nlohmann::json::parse(projectFile);
		if (j.contains("projectName")) {
			projectName = j["projectName"];
		}
	}

	openMainWindow(projectDirectory, projectName);
}

void ProjectWindow::openMainWindow(const std::string& projectDirectory, const std::string& projectName) {
	nlohmann::json j;

	std::fstream recentProjectsFile("recent_projects.json", std::ios::in);
	if (recentProjectsFile.is_open()) {
		if (nlohmann::json::accept(recentProjectsFile)) {
			recentProjectsFile = std::fstream("recent_projects.json", std::ios::in);
			j = nlohmann::json::parse(recentProjectsFile);
		}
	}

	recentProjectsFile = std::fstream("recent_projects.json", std::ios::out | std::ios::trunc);
	if (!j.contains("projects")) {
		j["projects"] = { projectDirectory };
	}
	else {
		for (size_t i = 0; i < j["projects"].size(); i++) {
			const auto& project = j["projects"][i];
			if (project == projectDirectory) {
				j["projects"].erase(i);
			}
		}
		j["projects"].insert(j["projects"].begin(), projectDirectory);
	}

	recentProjectsFile << j.dump(1, '\t');
	recentProjectsFile.close();

	std::fstream projectFile(projectDirectory + "/project.ntpj", std::ios::in);
	if (projectFile.is_open()) {
		j = nlohmann::json::parse(projectFile);
		if (j.contains("steamAppID")) {
			m_globalInfo.steamAppID = j["steamAppID"];
		}
	}

	m_globalInfo.projectDirectory = projectDirectory;
	m_globalInfo.rendererResourceManager.projectDirectory = projectDirectory;
	m_globalInfo.projectName = projectName;
	m_globalInfo.mainWindow = new MainWindow(m_globalInfo);
	MainWindow* mainWindow = m_globalInfo.mainWindow;
	m_globalInfo.logger.addLog(LogLevel::Info, m_globalInfo.localization.getString("log_opened_project", { projectName }));
	mainWindow->show();
	close();
}