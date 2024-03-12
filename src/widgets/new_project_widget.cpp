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
	std::string projectNameNoSpace = m_projectName;
	std::replace(projectNameNoSpace.begin(), projectNameNoSpace.end(), ' ', '_');

	if (m_projectName != "") {
		m_projectPathLabel->setText(QString::fromStdString(m_projectDirectoryPath + "/" + projectNameNoSpace));
	}
	else {
		m_projectPathLabel->setText(QString::fromStdString(m_projectDirectoryPath + "/" + projectNameNoSpace));
		m_createNewProjectButton->setEnabled(false);
		return;
	}

	bool directoryExists = std::filesystem::exists(m_projectDirectoryPath + "/" + projectNameNoSpace);
	if (directoryExists) {
		m_projectPathLabel->setText(m_projectPathLabel->text() + " (directory already exists)");
	}

	if (((m_projectDirectoryPath != "") && (m_projectName != "")) && !directoryExists) {
		m_createNewProjectButton->setEnabled(true);
	}
	else {
		m_createNewProjectButton->setEnabled(false);
	}
}

void NewProjectWidget::onTextChanged(const std::string& text) {
	m_projectName = text;
	std::string projectNameNoSpace = m_projectName;
	std::replace(projectNameNoSpace.begin(), projectNameNoSpace.end(), ' ', '_');

	if (m_projectDirectoryPath != "") {
		if (m_projectName != "") {
			m_projectPathLabel->setText(QString::fromStdString(m_projectDirectoryPath + "/" + projectNameNoSpace));
		}
		else {
			m_projectPathLabel->setText(QString::fromStdString(m_projectDirectoryPath + "/? (missing project name)"));
			m_createNewProjectButton->setEnabled(false);
			return;
		}
	}
	else {
		if (m_projectName != "") {
			m_projectPathLabel->setText(QString("?/") + QString::fromStdString(projectNameNoSpace) + " (missing directory)");
		}
		else {
			m_projectPathLabel->setText(QString::fromStdString(m_projectDirectoryPath + "?/? (missing directory and project name)"));
		}
		return;
	}

	bool directoryExists = std::filesystem::exists(m_projectDirectoryPath + "/" + projectNameNoSpace);
	if (directoryExists) {
		m_projectPathLabel->setText(m_projectPathLabel->text() + " (directory already exists)");
	}

	if (((m_projectDirectoryPath != "") && (m_projectName != "")) && !directoryExists) {
		m_createNewProjectButton->setEnabled(true);
	}
	else {
		m_createNewProjectButton->setEnabled(false);
	}
}