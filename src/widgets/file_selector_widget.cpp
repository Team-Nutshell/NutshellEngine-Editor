#include "file_selector_widget.h"
#include <QHBoxLayout>
#include <QFileDialog>
#include <algorithm>

FileSelectorWidget::FileSelectorWidget(GlobalInfo& globalInfo, const std::string& noFileText, const std::string& buttonText, const std::string& defaultPath) : m_globalInfo(globalInfo), m_defaultPath(defaultPath) {
	setLayout(new QHBoxLayout());
	layout()->setContentsMargins(0, 0, 0, 0);
	filePathLabel = new QLabel(QString::fromStdString(noFileText));
	layout()->addWidget(filePathLabel);
	filePathButton = new QPushButton(QString::fromStdString(buttonText));
	layout()->addWidget(filePathButton);
	layout()->setAlignment(filePathButton, Qt::AlignmentFlag::AlignRight);

	connect(filePathButton, &QPushButton::clicked, this, &FileSelectorWidget::onFilePathButtonClicked);
}

void FileSelectorWidget::onFilePathButtonClicked() {
	QFileDialog fileDialog = QFileDialog();
	fileDialog.setWindowTitle("NutshellEngine - " + filePathButton->text());
	fileDialog.setWindowIcon(QIcon("assets/icon.png"));
	std::string filePathDirectory = filePath.substr(0, filePath.rfind('/'));
	if (filePath != "") {
		if (std::filesystem::path(filePath).is_absolute()) {
			fileDialog.setDirectory(QString::fromStdString(filePathDirectory));
		}
		else {
			fileDialog.setDirectory(QString::fromStdString(m_globalInfo.projectDirectory + "/" + filePathDirectory));
		}
	}
	else {
		fileDialog.setDirectory(QString::fromStdString(m_defaultPath));
	}

	if (fileDialog.exec()) {
		filePath = fileDialog.selectedFiles()[0].toStdString();
		filePathLabel->setText(QString::fromStdString(filePath.substr(filePath.rfind('/') + 1)));
		filePathLabel->setToolTip(QString::fromStdString(filePath));
		emit fileSelected(filePath);
	}
}