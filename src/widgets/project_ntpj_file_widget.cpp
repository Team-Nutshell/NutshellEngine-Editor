#include "project_ntpj_file_widget.h"
#include "main_window.h"
#include "../../external/nlohmann/json.hpp"
#include <QVBoxLayout>
#include <fstream>

ProjectNtpjFileWidget::ProjectNtpjFileWidget(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	resize(640, 360);
	setWindowTitle("NutshellEngine - Project Settings");
	setWindowIcon(QIcon("assets/icon.png"));
	setAttribute(Qt::WA_DeleteOnClose);

	setLayout(new QVBoxLayout());
	layout()->setAlignment(Qt::AlignmentFlag::AlignTop);
	projectNameWidget = new StringWidget(m_globalInfo, "Project Name");
	layout()->addWidget(projectNameWidget);

	connect(projectNameWidget, &StringWidget::valueChanged, this, &ProjectNtpjFileWidget::onValueChanged);

	std::string projectFilePath = m_globalInfo.projectDirectory + "/project.ntpj";

	std::fstream projectFile(projectFilePath, std::ios::in);
	if (projectFile.is_open()) {
		if (!nlohmann::json::accept(projectFile)) {
			m_globalInfo.logger.addLog(LogLevel::Warning, "\"" + projectFilePath + "\" is not a valid JSON file.");
			return;
		}
	}
	else {
		std::ofstream newProjectFile(projectFilePath);
		newProjectFile << "{\n}";
		newProjectFile.close();
	}

	projectFile = std::fstream(projectFilePath, std::ios::in);
	nlohmann::json j = nlohmann::json::parse(projectFile);

	if (j.contains("projectName")) {
		projectNameWidget->setText(j["projectName"]);
	}
	else {
		m_globalInfo.projectName = m_globalInfo.projectDirectory.substr(m_globalInfo.projectDirectory.rfind('/') + 1);
		projectNameWidget->setText(m_globalInfo.projectName);
	}
}

void ProjectNtpjFileWidget::onValueChanged() {
	if (projectNameWidget->getText() != "") {
		m_globalInfo.projectName = projectNameWidget->getText();
	}
	else {
		m_globalInfo.projectName = m_globalInfo.projectDirectory.substr(m_globalInfo.projectDirectory.rfind('/') + 1);
		projectNameWidget->setText(m_globalInfo.projectName);
	}

	std::string mainWindowTitle = "";
	if (m_globalInfo.mainWindow->windowTitle().toStdString()[0] == '*') {
		mainWindowTitle += '*';
	}
	mainWindowTitle += "NutshellEngine - " + m_globalInfo.projectName;
	if (!m_globalInfo.currentScenePath.empty()) {
		mainWindowTitle += " - " + m_globalInfo.currentScenePath;
	}
	m_globalInfo.mainWindow->setWindowTitle(QString::fromStdString(mainWindowTitle));

	save();
}

void ProjectNtpjFileWidget::save() {
	std::string projectFilePath = m_globalInfo.projectDirectory + "/project.ntpj";

	nlohmann::json j;
	if (projectNameWidget->getText() != "") {
		j["projectName"] = projectNameWidget->getText();
	}

	std::fstream projectFile(projectFilePath, std::ios::out | std::ios::trunc);
	if (j.empty()) {
		projectFile << "{\n}";
	}
	else {
		projectFile << j.dump(1, '\t');
	}
}
