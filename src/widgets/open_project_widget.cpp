#include "open_project_widget.h"
#include <QHBoxLayout>
#include <filesystem>

OpenProjectWidget::OpenProjectWidget(GlobalInfo& globalInfo): m_globalInfo(globalInfo) {
	setLayout(new QHBoxLayout());
	layout()->setContentsMargins(0, 0, 0, 0);
	m_directoryPathButton = new FilePushButton(m_globalInfo, "Select a project directory", "", FilePushButton::PathType::Directory);
	layout()->addWidget(m_directoryPathButton);
	connect(m_directoryPathButton, &FilePushButton::pathChanged, this, &OpenProjectWidget::onPathChanged);
}

void OpenProjectWidget::onPathChanged(const std::string& path) {
	std::string projectDirectoryPath = std::filesystem::canonical(path).string();
	std::replace(projectDirectoryPath.begin(), projectDirectoryPath.end(), '\\', '/');
	emit projectDirectorySelected(projectDirectoryPath);
}