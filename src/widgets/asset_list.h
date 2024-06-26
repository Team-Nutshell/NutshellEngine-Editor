#pragma once
#include "../common/common.h"
#include <QListWidget>
#include <QFileSystemWatcher>
#include <QMouseEvent>
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
	void onItemDoubleClicked(QListWidgetItem* item);
	void onCurrentTextChanged(const QString& currentText);
	void onDirectoryChanged(const QString& path);

	void mouseMoveEvent(QMouseEvent* event);
	void dragEnterEvent(QDragEnterEvent* event);
	void dragMoveEvent(QDragMoveEvent* event);
	void dropEvent(QDropEvent* event);

private:
	GlobalInfo& m_globalInfo;

	std::string m_assetsDirectory;
	std::string m_currentDirectory;
	QFileSystemWatcher m_directoryWatcher;
};