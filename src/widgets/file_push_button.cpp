#include "file_push_button.h"
#include "../common/asset_helper.h"
#include <QFileDialog>
#include <QMimeData>

FilePushButton::FilePushButton(GlobalInfo& globalInfo, const std::string& text, const std::string& defaultPath, PathType pathType) : QPushButton(QString::fromStdString(text)), m_globalInfo(globalInfo), m_defaultPath(defaultPath), m_pathType(pathType) {
	setAcceptDrops(true);
	
	connect(this, &QPushButton::clicked, this, &FilePushButton::onButtonClicked);
}

void FilePushButton::onButtonClicked() {
	QFileDialog fileDialog = QFileDialog();
	fileDialog.setWindowTitle("NutshellEngine - " + text());
	fileDialog.setWindowIcon(QIcon("assets/icon.png"));
	if (m_pathType == PathType::Directory) {
		fileDialog.setFileMode(QFileDialog::FileMode::Directory);
	}
	if (!path.empty()) {
		std::string pathDirectory;
		if (std::filesystem::is_directory(path)) {
			pathDirectory = path;
		}
		else {
			pathDirectory = path.substr(0, path.rfind('/'));
		}

		if (std::filesystem::path(path).is_absolute()) {
			fileDialog.setDirectory(QString::fromStdString(pathDirectory));
		}
		else {
			fileDialog.setDirectory(QString::fromStdString(AssetHelper::relativeToAbsolute(pathDirectory, m_globalInfo.projectDirectory)));
		}
	}
	else {
		if (!m_defaultPath.empty()) {
			fileDialog.setDirectory(QString::fromStdString(m_defaultPath));
		}
	}

	if (fileDialog.exec()) {
		path = fileDialog.selectedFiles()[0].toStdString();
		emit pathChanged(path);
	}
}

void FilePushButton::dragEnterEvent(QDragEnterEvent* event) {
	if (event->mimeData()->hasUrls()) {
		event->acceptProposedAction();
	}
}

void FilePushButton::dragMoveEvent(QDragMoveEvent* event) {
	if (event->mimeData()->hasUrls()) {
		event->acceptProposedAction();
	}
}

void FilePushButton::dropEvent(QDropEvent* event) {
	QList<QUrl> sources = event->mimeData()->urls();
	if (!sources.isEmpty()) {
		path = sources[0].toLocalFile().toStdString();
		bool pathIsFile = !std::filesystem::is_directory(path);
		if (((m_pathType == PathType::File) && !pathIsFile) || ((m_pathType == PathType::Directory) && pathIsFile)) {
			return;
		}
		emit pathChanged(path);
	}
}
