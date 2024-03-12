#include "file_selector_widget.h"
#include <QHBoxLayout>
#include <QFileDialog>
#include <algorithm>

FileSelectorWidget::FileSelectorWidget(GlobalInfo& globalInfo, const std::string& noFileText, const std::string& buttonText) : m_globalInfo(globalInfo) {
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
	fileDialog.setWindowTitle(filePathButton->text());
	std::string filePathDirectory = m_filePath.substr(0, m_filePath.rfind('/'));
	if (m_filePath != "") {
		fileDialog.setDirectory(QString::fromStdString(filePathDirectory));
	}
	else if (m_globalInfo.projectDirectory != "") {
		fileDialog.setDirectory(QString::fromStdString(m_globalInfo.projectDirectory));
	}

	if (fileDialog.exec()) {
		m_filePath = fileDialog.selectedFiles()[0].toStdString();
		filePathLabel->setText(QString::fromStdString(m_filePath.substr(m_filePath.rfind('/') + 1)));
		filePathLabel->setToolTip(QString::fromStdString(m_filePath));
		emit fileSelected(m_filePath);
	}
}