#include "project_window.h"
#include "separator_line.h"
#include "main_window.h"
#include "../../external/nlohmann/json.hpp"
#include <QVBoxLayout>
#include <QIcon>
#include <QPixmap>
#include <fstream>
#include <algorithm>

ProjectWindow::ProjectWindow(GlobalInfo& globalInfo): m_globalInfo(globalInfo) {
	setFixedSize(550, 400);
	setWindowTitle("NutshellEngine");
	setWindowIcon(QIcon("assets/icon.png"));

	setLayout(new QVBoxLayout());
	layout()->setAlignment(Qt::AlignmentFlag::AlignCenter);
	m_logoLabel = new QLabel();
	m_logoPixmap = new QPixmap("assets/logo.png");
	m_logoLabel->setPixmap(m_logoPixmap->scaled(width() - 20, height(), Qt::AspectRatioMode::KeepAspectRatio, Qt::TransformationMode::SmoothTransformation));
	layout()->addWidget(m_logoLabel);
	layout()->addWidget(new SeparatorLine(globalInfo));
	layout()->addWidget(new QLabel("<b>Open Project:</b>"));
	m_openProjectWidget = new OpenProjectWidget(m_globalInfo);
	layout()->addWidget(m_openProjectWidget);
	layout()->addWidget(new SeparatorLine(globalInfo));
	layout()->addWidget(new QLabel("<b>New Project:</b>"));
	m_newProjectWidget = new NewProjectWidget(m_globalInfo);
	layout()->addWidget(m_newProjectWidget);

	connect(m_openProjectWidget, &OpenProjectWidget::projectDirectorySelected, this, &ProjectWindow::onProjectDirectorySelected);
	connect(m_newProjectWidget, &NewProjectWidget::newProjectButtonClicked, this, &ProjectWindow::onNewProjectButtonClicked);
}

void ProjectWindow::onNewProjectButtonClicked(const std::string& projectDirectory, const std::string& projectName) {
	std::filesystem::create_directory(projectDirectory);
	std::filesystem::copy("assets/base_project", projectDirectory, std::filesystem::copy_options::recursive);

	std::fstream optionsFile(projectDirectory + "/assets/options/options.ntop", std::ios::out | std::ios::trunc);
	if (optionsFile.is_open()) {
		const std::string optionsFileContent = "{\n\t\"windowTitle\": \"" + projectName + "\"\n}";
		optionsFile << optionsFileContent;
	}

	nlohmann::json j;
	j["projectName"] = projectName;
	std::fstream projectFile(projectDirectory + "/project.ntpj", std::ios::out | std::ios::trunc);
	projectFile << j.dump(1, '\t');

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
	m_globalInfo.projectDirectory = projectDirectory;
	m_globalInfo.rendererResourceManager.projectDirectory = projectDirectory;
	m_globalInfo.projectName = projectName;
	m_globalInfo.mainWindow = new MainWindow(m_globalInfo);
	MainWindow* mainWindow = m_globalInfo.mainWindow;
	m_globalInfo.logger.addLog(LogLevel::Info, "Opened project " + projectName + ".");
	mainWindow->show();
	close();
}