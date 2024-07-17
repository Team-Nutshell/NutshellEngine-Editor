#pragma once
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

signals:
	void directoryChanged(const std::string& directory);

private slots:
	void onItemClicked(QListWidgetItem* item);
	void onItemDoubleClicked(QListWidgetItem* item);
	void onDirectoryChanged(const QString& path);

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
};