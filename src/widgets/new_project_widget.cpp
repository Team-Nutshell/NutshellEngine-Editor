#include "new_project_widget.h"
#include <QVBoxLayout>
#include <filesystem>

NewProjectWidget::NewProjectWidget(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setLayout(new QVBoxLayout());
	layout()->setContentsMargins(0, 0, 0, 0);
	m_newProjectDirectoryPathWidget = new NewProjectDirectoryPathWidget(m_globalInfo);
	layout()->addWidget(m_newProjectDirectoryPathWidget);
	m_newProjectNameWidget = new NewProjectNameWidget(m_globalInfo);
	layout()->addWidget(m_newProjectNameWidget);
	m_projectPathLabel = new QLabel("?/? (" + QString::fromStdString(m_globalInfo.localization.getString("new_project_missing_directory_project_name")) + ")");
	layout()->addWidget(m_projectPathLabel);
	m_createNewProjectButton = new QPushButton(QString::fromStdString(m_globalInfo.localization.getString("new_project_create")));
	m_createNewProjectButton->setEnabled(false);
	layout()->addWidget(m_createNewProjectButton);

	connect(m_createNewProjectButton, &QPushButton::clicked, this, &NewProjectWidget::onCreateNewProjectButtonClicked);
	connect(m_newProjectDirectoryPathWidget, &NewProjectDirectoryPathWidget::newProjectDirectorySelected, this, &NewProjectWidget::onDirectorySelected);
	connect(m_newProjectNameWidget, &NewProjectNameWidget::textChanged, this, &NewProjectWidget::onTextChanged);
}

void NewProjectWidget::onCreateNewProjectButtonClicked() {
	std::replace(m_projectDirectoryPath.begin(), m_projectDirectoryPath.end(), '\\', '/');
	std::string projectNameNoSpace = m_projectName;
	std::replace(projectNameNoSpace.begin(), projectNameNoSpace.end(), ' ', '_');
	emit newProjectButtonClicked(m_projectDirectoryPath + "/" + projectNameNoSpace, m_projectName);
}

void NewProjectWidget::onDirectorySelected(const std::string& directoryPath) {
	m_projectDirectoryPath = directoryPath;
	std::replace(m_projectDirectoryPath.begin(), m_projectDirectoryPath.end(), '\\', '/');

	std::string projectNameNoSpace = m_projectName;
	std::replace(projectNameNoSpace.begin(), projectNameNoSpace.end(), ' ', '_');

	std::string projectFullPath;
	if (!m_projectName.empty()) {
		projectFullPath = m_projectDirectoryPath + "/" + projectNameNoSpace;
	}
	else {
		projectFullPath = m_projectDirectoryPath + "/? (" + m_globalInfo.localization.getString("new_project_missing_project_name") + ")";
	}

	if ((!m_projectDirectoryPath.empty()) && (!m_projectName.empty())) {
		bool directoryExists = std::filesystem::exists(m_projectDirectoryPath + "/" + projectNameNoSpace);
		if (directoryExists) {
			projectFullPath = projectFullPath + " (" + m_globalInfo.localization.getString("new_project_directory_already_exists") + ")";

			m_createNewProjectButton->setEnabled(false);
		}
		else {
			m_createNewProjectButton->setEnabled(true);
		}
	}
	else {
		m_createNewProjectButton->setEnabled(false);
	}

	m_projectPathLabel->setText(QString::fromStdString(projectFullPath));
}

void NewProjectWidget::onTextChanged(const std::string& text) {
	m_projectName = text;
	std::string projectNameNoSpace = m_projectName;
	std::replace(projectNameNoSpace.begin(), projectNameNoSpace.end(), ' ', '_');

	std::string projectFullPath;
	if (!m_projectDirectoryPath.empty()) {
		if (!m_projectName.empty()) {
			projectFullPath = m_projectDirectoryPath + "/" + projectNameNoSpace;
		}
		else {
			projectFullPath = m_projectDirectoryPath + "/? (" + m_globalInfo.localization.getString("new_project_missing_project_name") + ")";
		}
	}
	else {
		if (!m_projectName.empty()) {
			projectFullPath = "?/" + projectNameNoSpace + " (" + m_globalInfo.localization.getString("new_project_missing_directory") + ")";
		}
		else {
			projectFullPath = "?/? (" + m_globalInfo.localization.getString("new_project_missing_directory_project_name") + ")";
		}
	}

	if ((!m_projectDirectoryPath.empty()) && (!m_projectName.empty())) {
		bool directoryExists = std::filesystem::exists(m_projectDirectoryPath + "/" + projectNameNoSpace);
		if (directoryExists) {
			projectFullPath = projectFullPath + " (" + m_globalInfo.localization.getString("new_project_directory_already_exists") + ")";

			m_createNewProjectButton->setEnabled(false);
		}
		else {
			m_createNewProjectButton->setEnabled(true);
		}
	}
	else {
		m_createNewProjectButton->setEnabled(false);
	}

	m_projectPathLabel->setText(QString::fromStdString(projectFullPath));
}