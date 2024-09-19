#include "asset_list.h"
#include "image_viewer.h"
#include "asset_info_name_widget.h"
#include "delete_asset_widget.h"
#include "close_scene_widget.h"
#include "main_window.h"
#include "../common/scene_manager.h"
#include <QSizePolicy>
#include <QSignalBlocker>
#include <QImage>
#include <filesystem>
#include <algorithm>
#include <fstream>
#include <regex>

AssetList::AssetList(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	if (!std::filesystem::exists(m_globalInfo.projectDirectory + "/assets/")) {
		return;
	}
	m_assetsDirectory = std::filesystem::canonical(m_globalInfo.projectDirectory + "/assets/").string();
	std::replace(m_assetsDirectory.begin(), m_assetsDirectory.end(), '\\', '/');
	m_currentDirectory = m_assetsDirectory;

	setWrapping(true);
	setAcceptDrops(true);
	setDragDropMode(QListWidget::DragDrop);
	setResizeMode(QListWidget::Adjust);
	menu = new AssetListMenu(m_globalInfo);
	setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);

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

	connect(this, &QListWidget::customContextMenuRequested, this, &AssetList::showMenu);
	connect(this, &QListWidget::itemClicked, this, &AssetList::onItemClicked);
	connect(this, &QListWidget::itemDoubleClicked, this, &AssetList::onItemDoubleClicked);
	connect(&m_directoryWatcher, &QFileSystemWatcher::directoryChanged, this, &AssetList::onDirectoryChanged);
	connect(&m_globalInfo.signalEmitter, &SignalEmitter::renameFileSignal, this, &AssetList::onFileRenamed);
	connect(itemDelegate(), &QAbstractItemDelegate::closeEditor, this, &AssetList::onLineEditClose);
}

void AssetList::enterDirectory(const std::string& directory) {
	m_currentDirectory = directory;
	if (!m_directoryWatcher.directories().empty()) {
		m_directoryWatcher.removePaths(m_directoryWatcher.directories());
	}
	m_directoryWatcher.addPath(QString::fromStdString(m_currentDirectory));

	updateAssetList();

	setCurrentRow(0);

	emit directoryChanged(m_currentDirectory.substr(m_globalInfo.projectDirectory.size() + 1));
}

void AssetList::actionOnFile(const std::string& file) {
	size_t lastDot = file.rfind('.');
	if (lastDot != std::string::npos) {
		std::string extension = file.substr(lastDot + 1);

		if (extension == "ntsn") {
			if ((m_globalInfo.mainWindow->windowTitle()[0] == '*') && !m_globalInfo.currentScenePath.empty()) {
				CloseSceneWidget* closeSceneWidget = new CloseSceneWidget(m_globalInfo);
				closeSceneWidget->show();
				m_openScenePath = file;

				connect(closeSceneWidget, &CloseSceneWidget::confirmSignal, this, &AssetList::onCloseSceneConfirmed);
			}
			else {
				SceneManager::openScene(m_globalInfo, m_currentDirectory + "/" + file);
			}
		}
		else if ((extension == "jpg") || (extension == "jpeg") || (extension == "png") || (extension == "ntim")) {
			ImageViewer* imageViewer = new ImageViewer(m_globalInfo, m_currentDirectory + "/" + file);
			imageViewer->show();
		}
	}
}

void AssetList::updateAssetList() {
	{
		const QSignalBlocker signalBlocker(this);
		clear();
	}

	if (!std::filesystem::equivalent(m_currentDirectory, m_assetsDirectory)) {
		addItem("../");
	}

	std::vector<std::string> directoryNames;
	std::vector<std::string> fileNames;
	for (const auto& entry : std::filesystem::directory_iterator(m_currentDirectory)) {
		std::string entryPath = entry.path().string();
		std::replace(entryPath.begin(), entryPath.end(), '\\', '/');

		if (std::filesystem::is_directory(entry)) {
			directoryNames.push_back(entryPath.substr(entryPath.find_last_of('/') + 1) + '/');
		}
		else {
			fileNames.push_back(entryPath.substr(entryPath.find_last_of('/') + 1));
		}
	}

	std::sort(directoryNames.begin(), directoryNames.end());
	std::sort(fileNames.begin(), fileNames.end());

	for (const std::string& directoryName : directoryNames) {
		addItem(QString::fromStdString(directoryName));
	}
	for (const std::string& fileName : fileNames) {
		addItem(QString::fromStdString(fileName));
	}
}

void AssetList::onItemClicked(QListWidgetItem* listWidgetItem) {
	std::string itemFileName = listWidgetItem->text().toStdString();

	if (std::filesystem::exists(m_currentDirectory + "/" + itemFileName)) {
		std::string selectedElementPath = std::filesystem::canonical(m_currentDirectory + "/" + itemFileName).string();
		std::replace(selectedElementPath.begin(), selectedElementPath.end(), '\\', '/');

		emit m_globalInfo.signalEmitter.selectAssetSignal(selectedElementPath);
	}
}

void AssetList::onItemDoubleClicked(QListWidgetItem* listWidgetItem) {
	std::string itemFileName = listWidgetItem->text().toStdString();

	if (std::filesystem::exists(m_currentDirectory + "/" + itemFileName)) {
		std::string selectedElementPath = std::filesystem::canonical(m_currentDirectory + "/" + itemFileName).string();
		std::replace(selectedElementPath.begin(), selectedElementPath.end(), '\\', '/');

		if (!std::filesystem::is_directory(selectedElementPath)) {
			actionOnFile(itemFileName);
		}
		else {
			enterDirectory(selectedElementPath);
		}
	}
}

void AssetList::onDirectoryChanged() {
	updateAssetList();
}

void AssetList::onFileRenamed(const std::string& oldFilename, const std::string& newFilename) {
	if (oldFilename == m_globalInfo.currentScenePath) {
		m_globalInfo.currentScenePath = newFilename;
		m_globalInfo.mainWindow->updateTitle();
	}
}

void AssetList::onCloseSceneConfirmed() {
	SceneManager::openScene(m_globalInfo, m_currentDirectory + "/" + m_openScenePath);
}

void AssetList::showMenu(const QPoint& pos) {
	QListWidgetItem* item = itemAt(pos);
	if (!item) {
		menu->directory = m_currentDirectory;
		menu->renameAction->setEnabled(false);
		menu->deleteAction->setEnabled(false);
	}
	else {
		if (item->text() != "../") {
			menu->directory = m_currentDirectory;
			menu->filename = item->text().toStdString();
			menu->renameAction->setEnabled(true);
			menu->deleteAction->setEnabled(true);
			emit m_globalInfo.signalEmitter.selectAssetSignal(m_currentDirectory + "/" + item->text().toStdString());
		}
		else {
			menu->directory = m_currentDirectory;
			menu->renameAction->setEnabled(false);
			menu->deleteAction->setEnabled(false);
		}
	}
	menu->popup(QCursor::pos());
}

QStringList AssetList::mimeTypes() const {
	return QStringList("text/uri-list");
}

QMimeData* AssetList::mimeData(const QList<QListWidgetItem*>& items) const {
	QMimeData* itemMimeData = new QMimeData();
	QList<QUrl> urls;
	std::string path = m_currentDirectory + "/" + items[0]->text().toStdString();
	QUrl url = QUrl::fromLocalFile(QString::fromStdString(path));
	urls.append(url);
	itemMimeData->setUrls(urls);

	return itemMimeData;
}

void AssetList::keyPressEvent(QKeyEvent* event) {
	if (!selectedItems().empty()) {
		QListWidgetItem* listItem = selectedItems()[0];
		int currentSelectionIndex = row(listItem);
		if (event->key() == Qt::Key_Up) {
			clearSelection();
			if (currentSelectionIndex == 0) {
				setCurrentItem(item(count() - 1));
			}
			else {
				setCurrentItem(item(currentSelectionIndex - 1));
			}
		}
		else if (event->key() == Qt::Key_Down) {
			clearSelection();
			if (currentSelectionIndex == (count() - 1)) {
				setCurrentItem(item(0));
			}
			else {
				setCurrentItem(item(currentSelectionIndex + 1));
			}
		}
		else if ((event->key() == Qt::Key_Return) || (event->key() == Qt::Key_Enter)) {
			if (!currentlyEditedItemName.empty()) {
				return;
			}

			std::string itemFileName = listItem->text().toStdString();
			std::string selectedElementPath = std::filesystem::canonical(m_currentDirectory + "/" + itemFileName).string();
			std::replace(selectedElementPath.begin(), selectedElementPath.end(), '\\', '/');

			if (std::filesystem::is_directory(selectedElementPath)) {
				enterDirectory(selectedElementPath);
			}
			else {
				actionOnFile(itemFileName);
			}
		}
		else if (event->key() == Qt::Key_F2) {
			if (listItem && (listItem->text() != "../")) {
				currentlyEditedItemName = listItem->text().toStdString();
				listItem->setFlags(listItem->flags() | Qt::ItemFlag::ItemIsEditable);
				editItem(listItem);
			}
		}
		else if (event->key() == Qt::Key_Delete) {
			std::string itemFileName = listItem->text().toStdString();
			DeleteAssetWidget* deleteAssetWidget = new DeleteAssetWidget(m_globalInfo, m_currentDirectory + "/" + itemFileName);
			deleteAssetWidget->show();
		}
	}
}

void AssetList::dragEnterEvent(QDragEnterEvent* event) {
	if (event->source() == this) {
		return;
	}

	if (event->mimeData()->hasUrls()) {
		event->acceptProposedAction();
	}
}

void AssetList::dragMoveEvent(QDragMoveEvent* event) {
	if (event->source() == this) {
		return;
	}

	if (event->mimeData()->hasUrls()) {
		event->acceptProposedAction();
	}
}

void AssetList::dropEvent(QDropEvent* event) {
	if (event->source() == this) {
		return;
	}

	QList<QUrl> sources = event->mimeData()->urls();
	std::string destination = m_currentDirectory;
	for (const QUrl& source : sources) {
		std::string sourcePath = source.toLocalFile().toStdString();
		std::string fullDestinationDirectory = destination;
		if (std::filesystem::is_directory(sourcePath)) {
			fullDestinationDirectory = destination + "/" + std::filesystem::path(sourcePath).filename().string();
			if (!std::filesystem::exists(fullDestinationDirectory)) {
				std::filesystem::create_directory(fullDestinationDirectory);
			}
		}
		std::filesystem::copy(sourcePath, fullDestinationDirectory, std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive);
	}
}

void AssetList::onLineEditClose(QWidget* lineEdit, QAbstractItemDelegate::EndEditHint hint) {
	(void)hint;
	QListWidgetItem* currentItem = selectedItems()[0];
	std::string newName = reinterpret_cast<QLineEdit*>(lineEdit)->text().toStdString();
	if (currentlyEditedItemName != newName) {
		const std::regex validFilenameRegex(R"(^[a-zA-Z0-9._ -]+$)");
		if (!std::regex_search(newName, validFilenameRegex)) {
			currentItem->setText(QString::fromStdString(currentlyEditedItemName));
			return;
		}

		if (newName.empty()) {
			currentItem->setText(QString::fromStdString(currentlyEditedItemName));
			return;
		}

		if (std::filesystem::exists(m_currentDirectory + "/" + newName)) {
			currentItem->setText(QString::fromStdString(currentlyEditedItemName));
			return;
		}

		if (std::filesystem::exists(m_currentDirectory + "/" + currentlyEditedItemName)) {
			std::filesystem::rename(m_currentDirectory + "/" + currentlyEditedItemName, m_currentDirectory + "/" + newName);
			emit m_globalInfo.signalEmitter.renameFileSignal(m_currentDirectory + "/" + currentlyEditedItemName, m_currentDirectory + "/" + newName);
			emit m_globalInfo.signalEmitter.selectAssetSignal(m_currentDirectory + "/" + newName);
		}
	}
	currentlyEditedItemName = "";
}
