#pragma once
#include "asset_list_menu.h"
#include "../common/global_info.h"
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

private:
	void enterDirectory(const std::string& directory);
	void actionOnFile(const std::string& file);

signals:
	void directoryChanged(const std::string& directory);

private slots:
	void onItemClicked(QListWidgetItem* listWidgetItem);
	void onItemDoubleClicked(QListWidgetItem* listWidgetItem);
	void onDirectoryChanged(const QString& path);

	void showMenu(const QPoint& pos);

	QStringList mimeTypes() const;
	QMimeData* mimeData(const QList<QListWidgetItem*>& items) const;

	void keyPressEvent(QKeyEvent* event);
	void dragEnterEvent(QDragEnterEvent* event);
	void dragMoveEvent(QDragMoveEvent* event);
	void dropEvent(QDropEvent* event);

private:
	GlobalInfo& m_globalInfo;

	std::string m_assetsDirectory;
	std::string m_currentDirectory;
	QFileSystemWatcher m_directoryWatcher;

	bool m_movingMouse = false;

public:
	AssetListMenu* menu;
};