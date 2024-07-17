#include "file_selector_widget.h"
#include <QHBoxLayout>
#include <QFileDialog>
#include <algorithm>
#include <filesystem>

FileSelectorWidget::FileSelectorWidget(GlobalInfo& globalInfo, const std::string& noFileText, const std::string& buttonText, const std::string& defaultPath) : m_globalInfo(globalInfo) {
	setLayout(new QHBoxLayout());
	layout()->setContentsMargins(0, 0, 0, 0);
	filePathLabel = new QLabel(QString::fromStdString(noFileText));
	layout()->addWidget(filePathLabel);
	std::string filePath = defaultPath;
	if (!std::filesystem::path(filePath).is_absolute()) {
		filePath = m_globalInfo.projectDirectory + "/" + filePath;
	}
	filePathButton = new FilePushButton(m_globalInfo, buttonText, filePath, FilePushButton::PathType::File);
	filePathButton->setAcceptDrops(true);
	layout()->addWidget(filePathButton);
	layout()->setAlignment(filePathButton, Qt::AlignmentFlag::AlignRight);

	connect(filePathButton, &FilePushButton::pathChanged, this, &FileSelectorWidget::onPathChanged);
}

void FileSelectorWidget::onPathChanged(const std::string& path) {
	filePathLabel->setText(QString::fromStdString(path.substr(path.rfind('/') + 1)));
	filePathLabel->setToolTip(QString::fromStdString(path));

	emit fileSelected(path);
}