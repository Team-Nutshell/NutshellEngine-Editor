#include "new_project_directory_path_widget.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QFileDialog>

NewProjectDirectoryPathWidget::NewProjectDirectoryPathWidget(GlobalInfo& globalInfo): m_globalInfo(globalInfo) {
	setLayout(new QHBoxLayout());
	layout()->setContentsMargins(0, 0, 0, 0);
	layout()->addWidget(new QLabel("New project directory:"));
	m_directoryPathButton = new QPushButton("Select a project directory");
	layout()->addWidget(m_directoryPathButton);
	connect(m_directoryPathButton, &QPushButton::clicked, this, &NewProjectDirectoryPathWidget::onDirectoryPathButtonClicked);
}

void NewProjectDirectoryPathWidget::onDirectoryPathButtonClicked() {
	QFileDialog fileDialog = QFileDialog();
	fileDialog.setWindowTitle("NutshellEngine - " + m_directoryPathButton->text());
	fileDialog.setWindowIcon(QIcon("assets/icon.png"));
	fileDialog.setFileMode(QFileDialog::FileMode::Directory);

	if (fileDialog.exec()) {
		std::string directoryPath = std::filesystem::canonical(fileDialog.directory().path().toStdString()).string();
		m_directoryPathButton->setText(QString::fromStdString(directoryPath));
		emit newProjectDirectorySelected(directoryPath);
	}
}