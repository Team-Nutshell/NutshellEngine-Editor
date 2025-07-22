#pragma once
#include "../common/global_info.h"
#include "asset_list_menu.h"
#include <QListWidget>
#include <QFileSystemWatcher>
#include <QMimeData>
#include <QKeyEvent>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <string>

class AssetList : public QListWidget {
	Q_OBJECT
public:
	AssetList(GlobalInfo& globalInfo);

	void openPath(const std::string& path);

	void deleteAsset(const std::string& path);
	void duplicateAsset(const std::string& path);
	void reloadAsset(const std::string& assetPath, const std::string& assetName);

private:
	void enterDirectory(const std::string& directory);
	void actionOnFile(const std::string& file);
	void updateAssetList();

signals:
	void directoryChanged(const std::string& directory);

private slots:
	void onItemDoubleClicked(QListWidgetItem* listWidgetItem);
	void onDirectoryChanged();

	void onFileRenamed(const std::string& oldFilename, const std::string& newFilename);

	void onCloseSceneConfirmed();

	void showMenu(const QPoint& pos);

	QStringList mimeTypes() const;
	QMimeData* mimeData(const QList<QListWidgetItem*>& items) const;

	void keyPressEvent(QKeyEvent* event);
	void dragEnterEvent(QDragEnterEvent* event);
	void dragMoveEvent(QDragMoveEvent* event);
	void dropEvent(QDropEvent* event);
	void onLineEditClose(QWidget* lineEdit, QAbstractItemDelegate::EndEditHint hint);

private:
	GlobalInfo& m_globalInfo;

	std::string m_assetsDirectory;
	std::string m_currentDirectory;
	QFileSystemWatcher m_directoryWatcher;

	std::string m_openScenePath;

public:
	AssetListMenu* menu;

	std::string currentlyEditedItemName = "";
};