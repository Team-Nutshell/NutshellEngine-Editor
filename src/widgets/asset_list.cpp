#include "asset_list.h"
#include "../common/scene_manager.h"
#include "image_viewer.h"
#include "material_file_widget.h"
#include "options_file_widget.h"
#include "sampler_file_widget.h"
#include <QSizePolicy>
#include <QSignalBlocker>
#include <QImage>
#include <filesystem>
#include <algorithm>
#include <fstream>

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

	connect(this, &QListWidget::itemClicked, this, &AssetList::onItemClicked);
	connect(this, &QListWidget::itemDoubleClicked, this, &AssetList::onItemDoubleClicked);
	connect(&m_directoryWatcher, &QFileSystemWatcher::directoryChanged, this, &AssetList::onDirectoryChanged);
}

void AssetList::enterDirectory(const std::string& directory) {
	m_currentDirectory = directory;
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

	setCurrentRow(0);

	emit directoryChanged(m_currentDirectory.substr(m_globalInfo.projectDirectory.size() + 1));
}

void AssetList::actionOnFile(const std::string& file) {
	size_t lastDot = file.rfind('.');
	if (lastDot != std::string::npos) {
		std::string extension = file.substr(lastDot + 1);

		if (extension == "ntsn") {
			SceneManager::openScene(m_globalInfo, m_currentDirectory + "/" + file);
		}
		else if ((extension == "jpg") || (extension == "jpeg") || (extension == "png")) {
			QImage image = QImage(QString::fromStdString(m_currentDirectory) + "/" + QString::fromStdString(file));
			ImageViewer* imageViewer = new ImageViewer(m_globalInfo, m_currentDirectory + "/" + file, image);
			imageViewer->show();
		}
		else if (extension == "ntim") {
			int width = 1;
			int height = 1;
			std::vector<uint8_t> pixelData;

			std::fstream imageFile(m_currentDirectory + "/" + file, std::ios::in);
			if (imageFile.is_open()) {
				if (!nlohmann::json::accept(imageFile)) {
					m_globalInfo.logger.addLog(LogLevel::Warning, "\"" + m_currentDirectory + "/" + file + "\" is not a valid JSON file.");
					return;
				}
			}
			else {
				m_globalInfo.logger.addLog(LogLevel::Warning, "\"" + m_currentDirectory + "/" + file + "\" cannot be opened.");
				return;
			}

			imageFile = std::fstream(m_currentDirectory + "/" + file, std::ios::in);

			nlohmann::json j = nlohmann::json::parse(imageFile);

			if (j.contains("width")) {
				width = static_cast<uint32_t>(j["width"]);
			}

			if (j.contains("height")) {
				height = static_cast<uint32_t>(j["height"]);
			}

			if (j.contains("data")) {
				for (size_t i = 0; i < j["data"].size(); i++) {
					pixelData.push_back(static_cast<uint8_t>(j["data"][i]));
				}
			}
			QImage image = QImage(pixelData.data(), width, height, QImage::Format_RGBA8888);
			ImageViewer* imageViewer = new ImageViewer(m_globalInfo, m_currentDirectory + "/" + file, image);
			imageViewer->show();
		}
		else if (extension == "ntml") {
			MaterialFileWidget* materialFileWidget = new MaterialFileWidget(m_globalInfo, m_currentDirectory + "/" + file);
			materialFileWidget->show();
		}
		else if (extension == "ntsp") {
			SamplerFileWidget* samplerFileWidget = new SamplerFileWidget(m_globalInfo, m_currentDirectory + "/" + file);
			samplerFileWidget->show();
		}
		else if (extension == "ntop") {
			OptionsFileWidget* optionsFileWidget = new OptionsFileWidget(m_globalInfo, m_currentDirectory + "/" + file);
			optionsFileWidget->show();
		}
	}
}

void AssetList::onItemClicked(QListWidgetItem* item) {
	std::string itemFileName = item->text().toStdString();

	if (std::filesystem::exists(m_currentDirectory + "/" + itemFileName)) {
		std::string selectedElementPath = std::filesystem::canonical(m_currentDirectory + "/" + itemFileName).string();
		std::replace(selectedElementPath.begin(), selectedElementPath.end(), '\\', '/');

		if (std::filesystem::is_directory(selectedElementPath)) {
			enterDirectory(selectedElementPath);
		}
	}
}

void AssetList::onItemDoubleClicked(QListWidgetItem* item) {
	std::string itemFileName = item->text().toStdString();

	if (std::filesystem::exists(m_currentDirectory + "/" + itemFileName)) {
		std::string selectedElementPath = std::filesystem::canonical(m_currentDirectory + "/" + itemFileName).string();
		std::replace(selectedElementPath.begin(), selectedElementPath.end(), '\\', '/');

		if (!std::filesystem::is_directory(selectedElementPath)) {
			actionOnFile(itemFileName);
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

	if (!std::filesystem::equivalent(m_currentDirectory, m_assetsDirectory)) {
		addItem("../");
	}

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

	emit directoryChanged(directoryPath.substr(m_globalInfo.projectDirectory.size() + 1));
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
		if (event->key() == Qt::Key::Key_Up) {
			clearSelection();
			if (currentSelectionIndex == 0) {
				setCurrentItem(item(count() - 1));
			}
			else {
				setCurrentItem(item(currentSelectionIndex - 1));
			}
		}
		else if (event->key() == Qt::Key::Key_Down) {
			clearSelection();
			if (currentSelectionIndex == (count() - 1)) {
				setCurrentItem(item(0));
			}
			else {
				setCurrentItem(item(currentSelectionIndex + 1));
			}
		}
		else if ((event->key() == Qt::Key::Key_Return) || (event->key() == Qt::Key::Key_Enter)) {
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