#include "new_project_directory_path_widget.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QFileDialog>

NewProjectDirectoryPathWidget::NewProjectDirectoryPathWidget(GlobalInfo& globalInfo): m_globalInfo(globalInfo) {
	setLayout(new QHBoxLayout());
	layout()->setContentsMargins(0, 0, 0, 0);
	layout()->addWidget(new QLabel("New project directory:"));
	m_directoryPathButton = new FilePushButton(m_globalInfo, "Select a directory", "", FilePushButton::PathType::Directory);
	layout()->addWidget(m_directoryPathButton);
	connect(m_directoryPathButton, &FilePushButton::pathChanged, this, &NewProjectDirectoryPathWidget::onPathChanged);
}

void NewProjectDirectoryPathWidget::onPathChanged(const std::string& path) {
	std::string directoryPath = std::filesystem::canonical(path).string();
	m_directoryPathButton->setText(QString::fromStdString(directoryPath));
	emit newProjectDirectorySelected(directoryPath);
}