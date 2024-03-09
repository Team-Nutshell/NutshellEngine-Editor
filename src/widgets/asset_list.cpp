#include "asset_list.h"
#include "../common/scene_manager.h"
#include <QSizePolicy>
#include <QSignalBlocker>
#include <QMimeData>
#include <filesystem>
#include <algorithm>

AssetList::AssetList(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	m_assetsDirectory = std::filesystem::canonical(m_globalInfo.projectDirectory + "/assets/").string();
	std::replace(m_assetsDirectory.begin(), m_assetsDirectory.end(), '\\', '/');
	m_currentDirectory = m_assetsDirectory;

	setWrapping(true);
	setAcceptDrops(true);

	QSizePolicy sizePolicy;
	sizePolicy.setHorizontalPolicy(QSizePolicy::Policy::Ignored);
	sizePolicy.setVerticalPolicy(QSizePolicy::Policy::Expanding);
	setSizePolicy(sizePolicy);
	if (std::filesystem::exists(m_assetsDirectory)) {
		for (const auto& entry : std::filesystem::directory_iterator(m_assetsDirectory)) {
			std::string entryPath = entry.path().string();
			std::replace(entryPath.begin(), entryPath.end(), '\\', '/');
			if (std::filesystem::is_directory(entry)) {
				addItem(QString::fromStdString(entryPath.substr(entryPath.find_last_of('/') + 1)) + "/");
			}
			else {
				addItem(QString::fromStdString(entryPath.substr(entryPath.find_last_of('/') + 1)));
			}
		}

		m_directoryWatcher.addPath(QString::fromStdString(m_assetsDirectory));
	}

	connect(this, &QListWidget::itemDoubleClicked, this, &AssetList::onItemDoubleClicked);
	connect(this, &QListWidget::currentTextChanged, this, &AssetList::onCurrentTextChanged);
	connect(&m_directoryWatcher, &QFileSystemWatcher::directoryChanged, this, &AssetList::onDirectoryChanged);
}

void AssetList::onItemDoubleClicked(QListWidgetItem* item) {
	std::string itemText = item->text().toStdString();
	size_t lastDot = itemText.rfind('.');
	if (lastDot != std::string::npos) {
		std::string extension = itemText.substr(lastDot + 1);

		if (extension == "ntsn") {
			SceneManager::openScene(m_globalInfo, m_currentDirectory + "/" + itemText);
		}
	}
}

void AssetList::onCurrentTextChanged(const QString& currentText) {
	if (m_firstSelection) {
		m_firstSelection = false;
		return;
	}

	std::string selectedElementPath = std::filesystem::canonical(m_currentDirectory + "/" + currentText.toStdString()).string();
	std::replace(selectedElementPath.begin(), selectedElementPath.end(), '\\', '/');

	if (std::filesystem::exists(selectedElementPath)) {
		if (!std::filesystem::is_directory(selectedElementPath)) {
			return;
		}

		m_currentDirectory = selectedElementPath;
		if (!m_directoryWatcher.directories().empty()) {
			m_directoryWatcher.removePaths(m_directoryWatcher.directories());
		}
		m_directoryWatcher.addPath(QString::fromStdString(m_currentDirectory));

		{
			const QSignalBlocker signalBlocker(this);
			clear();
		}

		if (!std::filesystem::equivalent(m_currentDirectory, m_assetsDirectory)) {
			addItem("../");
		}

		for (const auto& entry : std::filesystem::directory_iterator(m_currentDirectory)) {
			std::string entryPath = entry.path().string();
			std::replace(entryPath.begin(), entryPath.end(), '\\', '/');
			if (std::filesystem::is_directory(entry)) {
				addItem(QString::fromStdString(entryPath.substr(entryPath.find_last_of('/') + 1)) + "/");
			}
			else {
				addItem(QString::fromStdString(entryPath.substr(entryPath.find_last_of('/') + 1)));
			}
		}
	}
}

void AssetList::onDirectoryChanged(const QString& path) {
	{
		const QSignalBlocker signalBlocker(this);
		clear();
	}

	std::string directoryPath = std::filesystem::canonical(path.toStdString()).string();
	std::replace(directoryPath.begin(), directoryPath.end(), '\\', '/');
	if (std::filesystem::exists(directoryPath)) {
		for (const auto& entry : std::filesystem::directory_iterator(path.toStdString())) {
			std::string entryPath = entry.path().string();
			std::replace(entryPath.begin(), entryPath.end(), '\\', '/');
			if (std::filesystem::is_directory(entry)) {
				addItem(QString::fromStdString(entryPath.substr(entryPath.find_last_of('/') + 1)) + "/");
			}
			else {
				addItem(QString::fromStdString(entryPath.substr(entryPath.find_last_of('/') + 1)));
			}
		}
	}
}

void AssetList::mouseMoveEvent(QMouseEvent* event) {
	if (event->buttons() & Qt::MouseButton::LeftButton) {
		return;
	}
	event->accept();
}

void AssetList::dragEnterEvent(QDragEnterEvent* event) {
	if (event->mimeData()->hasUrls()) {
		event->acceptProposedAction();
	}
}

void AssetList::dragMoveEvent(QDragMoveEvent* event) {
	if (event->mimeData()->hasUrls()) {
		event->acceptProposedAction();
	}
}

void AssetList::dropEvent(QDropEvent* event) {
	QList<QUrl> sources = event->mimeData()->urls();
	std::string destination = m_currentDirectory;
	for (const QUrl& source : sources) {
		std::string sourcePath = source.toLocalFile().toStdString();
		std::filesystem::copy(sourcePath, destination);
	}
}