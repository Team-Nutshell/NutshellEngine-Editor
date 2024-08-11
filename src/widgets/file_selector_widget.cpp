#include "file_selector_widget.h"
#include <QHBoxLayout>
#include <QFileDialog>
#include <algorithm>
#include <filesystem>

FileSelectorWidget::FileSelectorWidget(GlobalInfo& globalInfo, const std::string& labelText, const std::string& noFileText, const std::string& defaultPath) : m_globalInfo(globalInfo), m_noFileText(noFileText) {
	setLayout(new QHBoxLayout());
	layout()->setContentsMargins(0, 0, 0, 0);
	filePathLabel = new QLabel(QString::fromStdString(labelText));
	layout()->addWidget(filePathLabel);
	std::string filePath = defaultPath;
	if (!std::filesystem::path(filePath).is_absolute()) {
		filePath = m_globalInfo.projectDirectory + "/" + filePath;
	}
	filePathButton = new FilePushButton(m_globalInfo, noFileText, filePath, FilePushButton::PathType::File);
	filePathButton->setAcceptDrops(true);
	layout()->addWidget(filePathButton);
	layout()->setAlignment(filePathButton, Qt::AlignmentFlag::AlignRight);
	resetFilePathButton = new QPushButton("X");
	resetFilePathButton->setFixedWidth(20);
	layout()->addWidget(resetFilePathButton);

	connect(filePathButton, &FilePushButton::pathChanged, this, &FileSelectorWidget::onPathChanged);
	connect(resetFilePathButton, &QPushButton::clicked, this, &FileSelectorWidget::onResetFilePathClicked);
}

void FileSelectorWidget::setPath(const std::string& path) {
	if (path == "") {
		m_path = "";
		filePathButton->path = "";
		filePathButton->setText(QString::fromStdString(m_noFileText));
		filePathButton->setToolTip("");
	}
	else {
		m_path = path;
		filePathButton->path = path;
		filePathButton->setText(QString::fromStdString(path.substr(path.rfind('/') + 1)));
		filePathButton->setToolTip(QString::fromStdString(path));
	}
}

const std::string& FileSelectorWidget::getPath() {
	return m_path;
}

void FileSelectorWidget::onPathChanged(const std::string& path) {
	if (m_path != path) {
		setPath(path);
		emit fileSelected(path);
	}
}

void FileSelectorWidget::onResetFilePathClicked() {
	if (m_path != "") {
		setPath("");
		emit fileSelected("");
	}
}