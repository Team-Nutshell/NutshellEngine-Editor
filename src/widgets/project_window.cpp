#include "project_window.h"
#include "separator_line.h"
#include "main_window.h"
#include "../../external/nlohmann/json.hpp"
#include <QVBoxLayout>
#include <QIcon>
#include <QPixmap>
#include <fstream>

ProjectWindow::ProjectWindow(GlobalInfo& globalInfo): m_globalInfo(globalInfo) {
	setFixedSize(550, 400);
	setWindowTitle("NutshellEngine");
	setWindowIcon(QIcon("assets/icon.png"));

	setLayout(new QVBoxLayout());
	layout()->setAlignment(Qt::AlignmentFlag::AlignCenter);
	m_logoLabel = std::make_unique<QLabel>();
	m_logoPixmap = std::make_unique<QPixmap>("assets/logo.png");
	m_logoLabel->setPixmap(m_logoPixmap->scaled(width() - 20, height(), Qt::AspectRatioMode::KeepAspectRatio, Qt::TransformationMode::SmoothTransformation));
	layout()->addWidget(m_logoLabel.get());
	layout()->addWidget(new SeparatorLine(globalInfo));
	layout()->addWidget(new QLabel("<b>Open Project:</b>"));
	m_openProjectWidget = std::make_unique<OpenProjectWidget>(m_globalInfo);
	layout()->addWidget(m_openProjectWidget.get());
	layout()->addWidget(new SeparatorLine(globalInfo));
	layout()->addWidget(new QLabel("<b>New Project:</b>"));
	m_newProjectWidget = std::make_unique<NewProjectWidget>(m_globalInfo);
	layout()->addWidget(m_newProjectWidget.get());

	connect(m_openProjectWidget.get(), &OpenProjectWidget::projectDirectorySelected, this, &ProjectWindow::onProjectDirectorySelected);
	connect(m_newProjectWidget.get(), &NewProjectWidget::newProjectButtonClicked, this, &ProjectWindow::onNewProjectButtonClicked);
}

void ProjectWindow::onNewProjectButtonClicked(const std::string& projectDirectory, const std::string& projectName) {
	std::filesystem::create_directory(projectDirectory);
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
	m_globalInfo.projectName = projectName;
	m_globalInfo.mainWindow = new MainWindow(m_globalInfo);
	MainWindow* mainWindow = reinterpret_cast<MainWindow*>(m_globalInfo.mainWindow);
	mainWindow->show();
	close();
}