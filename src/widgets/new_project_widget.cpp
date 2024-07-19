#include "new_project_widget.h"
#include <QVBoxLayout>
#include <filesystem>

NewProjectWidget::NewProjectWidget(GlobalInfo& globalInfo): m_globalInfo(globalInfo) {
	setLayout(new QVBoxLayout());
	layout()->setContentsMargins(0, 0, 0, 0);
	m_newProjectDirectoryPathWidget = new NewProjectDirectoryPathWidget(m_globalInfo);
	layout()->addWidget(m_newProjectDirectoryPathWidget);
	m_newProjectNameWidget = new NewProjectNameWidget(m_globalInfo);
	layout()->addWidget(m_newProjectNameWidget);
	m_projectPathLabel = new QLabel("?/? (missing directory and project name)");
	layout()->addWidget(m_projectPathLabel);
	m_createNewProjectButton = new QPushButton("Create new project");
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
	if (m_projectName != "") {
		projectFullPath = m_projectDirectoryPath + "/" + projectNameNoSpace;
	}
	else {
		projectFullPath = m_projectDirectoryPath + "/? (missing project name)";
	}

	if ((m_projectDirectoryPath != "") && (m_projectName != "")) {
		bool directoryExists = std::filesystem::exists(m_projectDirectoryPath + "/" + projectNameNoSpace);
		if (directoryExists) {
			projectFullPath = projectFullPath + " (directory already exists)";

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
	if (m_projectDirectoryPath != "") {
		if (m_projectName != "") {
			projectFullPath = m_projectDirectoryPath + "/" + projectNameNoSpace;
		}
		else {
			projectFullPath = m_projectDirectoryPath + "/? (missing project name)";
		}
	}
	else {
		if (m_projectName != "") {
			projectFullPath = "?/" + projectNameNoSpace + " (missing directory)";
		}
		else {
			projectFullPath = "?/? (missing directory and project name)";
		}
	}

	if ((m_projectDirectoryPath != "") && (m_projectName != "")) {
		bool directoryExists = std::filesystem::exists(m_projectDirectoryPath + "/" + projectNameNoSpace);
		if (directoryExists) {
			projectFullPath = projectFullPath + " (directory already exists)";

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