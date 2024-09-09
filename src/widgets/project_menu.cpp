#include "project_menu.h"
#include "main_window.h"
#include "project_ntpj_file_widget.h"
#include <fstream>

ProjectMenu::ProjectMenu(GlobalInfo& globalInfo) : QMenu("&Project"), m_globalInfo(globalInfo) {
	m_buildAction = addAction("Build and Run", this, &ProjectMenu::launchBuild);
	m_buildAction->setShortcut(QKeySequence::fromString("F5"));
	addSeparator();
	m_openProjectSettingsAction = addAction("Open Project Settings", this, &ProjectMenu::openProjectSettings);
	m_openProjectSettingsAction->setShortcut(QKeySequence::fromString("Ctrl+Shift+P"));
	addSeparator();
	m_importGlobalResources = addAction("Import global resources", this, &ProjectMenu::importGlobalResources);
	m_updateBaseProject = addAction("Update base project", this, &ProjectMenu::updateBaseProject);

	connect(&m_globalInfo.signalEmitter, &SignalEmitter::startBuildAndRunSignal, this, &ProjectMenu::onBuildRunExportStarted);
	connect(&m_globalInfo.signalEmitter, &SignalEmitter::endBuildAndRunSignal, this, &ProjectMenu::onBuildRunExportEnded);
}

void ProjectMenu::launchBuild() {
	m_globalInfo.mainWindow->buildBar->launchBuild();
}

void ProjectMenu::openProjectSettings() {
	ProjectNtpjFileWidget* projectNtpjFileWidget = new ProjectNtpjFileWidget(m_globalInfo);
	projectNtpjFileWidget->show();
}

void ProjectMenu::importGlobalResources() {
	std::filesystem::copy("assets/global_resources", m_globalInfo.projectDirectory, std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive);

	if (!std::filesystem::exists(m_globalInfo.projectDirectory + "/assets/options")) {
		std::filesystem::create_directory(m_globalInfo.projectDirectory + "/assets/options");
	}

	bool optionsOpen = false;
	std::fstream optionsFile = std::fstream(m_globalInfo.projectDirectory + "/assets/options/options.ntop", std::ios::in);
	if (optionsFile.is_open()) {
		optionsOpen = nlohmann::json::accept(optionsFile);
	}

	nlohmann::json j;
	if (optionsOpen) {
		optionsFile = std::fstream(m_globalInfo.projectDirectory + "/assets/options/options.ntop", std::ios::in);
		j = nlohmann::json::parse(optionsFile);
	}

	optionsFile = std::fstream(m_globalInfo.projectDirectory + "/assets/options/options.ntop", std::ios::out | std::ios::trunc);
	if (!j.contains("windowTitle")) {
		j["windowTitle"] = m_globalInfo.projectName;
	}
	if (!j.contains("maxFPS")) {
		j["maxFPS"] = 60;
	}
	if (!j.contains("firstScenePath")) {
		j["firstScenePath"] = "assets/scenes/default_scene.ntsn";
	}
	optionsFile << j.dump(1, '\t');
}

void ProjectMenu::updateBaseProject() {
	std::filesystem::copy("assets/base_project", m_globalInfo.projectDirectory, std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive);
}

void ProjectMenu::onBuildRunExportStarted() {
	m_buildAction->setEnabled(false);
}

void ProjectMenu::onBuildRunExportEnded() {
	m_buildAction->setEnabled(true);
}