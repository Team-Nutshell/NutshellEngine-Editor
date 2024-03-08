#include "open_project_widget.h"
#include <QHBoxLayout>
#include <filesystem>

OpenProjectWidget::OpenProjectWidget(GlobalInfo& globalInfo): m_globalInfo(globalInfo) {
	setLayout(new QHBoxLayout());
	layout()->setContentsMargins(0, 0, 0, 0);
	m_directoryPathButton = std::make_unique<QPushButton>("Select a project directory");
	layout()->addWidget(m_directoryPathButton.get());
	connect(m_directoryPathButton.get(), &QPushButton::clicked, this, &OpenProjectWidget::onDirectoryPathButtonClicked);
}

void OpenProjectWidget::onDirectoryPathButtonClicked() {
	std::unique_ptr<QFileDialog> fileDialog = std::make_unique<QFileDialog>();
	fileDialog->setWindowTitle(m_directoryPathButton->text());
	fileDialog->setFileMode(QFileDialog::FileMode::Directory);

	if (fileDialog->exec()) {
		std::string projectDirectoryPath = std::filesystem::canonical(fileDialog->directory().path().toStdString()).string();
		std::replace(projectDirectoryPath.begin(), projectDirectoryPath.end(), '\\', '/');
		emit projectDirectorySelected(projectDirectoryPath);
	}
}