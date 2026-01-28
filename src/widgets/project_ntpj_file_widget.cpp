#include "project_ntpj_file_widget.h"
#include "main_window.h"
#include "../../external/nlohmann/json.hpp"
#include <QVBoxLayout>
#include <fstream>

ProjectNtpjFileWidget::ProjectNtpjFileWidget(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setLayout(new QVBoxLayout());
	layout()->setAlignment(Qt::AlignmentFlag::AlignTop);
	layout()->setContentsMargins(5, 5, 5, 5);
	projectNameWidget = new StringWidget(m_globalInfo, m_globalInfo.localization.getString("project_settings_project_name"));
	projectNameWidget->setText(m_globalInfo.projectName);
	layout()->addWidget(projectNameWidget);
	steamAppIDWidget = new IntegerWidget(m_globalInfo, m_globalInfo.localization.getString("project_settings_steam_appid"));
	steamAppIDWidget->setValue(m_globalInfo.steamAppID);
	layout()->addWidget(steamAppIDWidget);

	connect(projectNameWidget, &StringWidget::valueChanged, this, &ProjectNtpjFileWidget::onStringChanged);
	connect(steamAppIDWidget, &IntegerWidget::valueChanged, this, &ProjectNtpjFileWidget::onIntegerChanged);

	std::string projectFilePath = m_globalInfo.projectDirectory + "/project.ntpj";

	std::fstream projectFile(projectFilePath, std::ios::in);
	if (projectFile.is_open()) {
		if (!nlohmann::json::accept(projectFile)) {
			m_globalInfo.logger.addLog(LogLevel::Warning, m_globalInfo.localization.getString("log_file_is_not_valid_json", { projectFilePath }));
			return;
		}
	}
	else {
		std::ofstream newProjectFile(projectFilePath);
		newProjectFile << "{\n}";
		newProjectFile.close();
	}
}

void ProjectNtpjFileWidget::onStringChanged(const std::string& value) {
	QObject* senderWidget = sender();

	if (senderWidget == projectNameWidget) {
		if (!value.empty()) {
			m_globalInfo.projectName = projectNameWidget->getText();
		}
		else {
			m_globalInfo.projectName = m_globalInfo.projectDirectory.substr(m_globalInfo.projectDirectory.rfind('/') + 1);
			projectNameWidget->setText(m_globalInfo.projectName);
		}
		m_globalInfo.mainWindow->updateTitle();
	}

	save();
}

void ProjectNtpjFileWidget::onIntegerChanged(int value) {
	QObject* senderWidget = sender();

	if (senderWidget == steamAppIDWidget) {
		m_globalInfo.steamAppID = value;
	}

	save();
}

void ProjectNtpjFileWidget::save() {
	std::fstream projectFile(m_globalInfo.projectDirectory + "/project.ntpj", std::ios::in);
	if (projectFile.is_open()) {
		nlohmann::json j = nlohmann::json::parse(projectFile);
		j["projectName"] = m_globalInfo.projectName;
		j["steamAppID"] = m_globalInfo.steamAppID;
		projectFile.close();

		projectFile = std::fstream(m_globalInfo.projectDirectory + "/project.ntpj", std::ios::out | std::ios::trunc);
		projectFile << j.dump(1, '\t');
	}
}
