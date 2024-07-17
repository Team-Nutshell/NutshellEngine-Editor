#pragma once
#include "../common/global_info.h"
#include <QPushButton>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <string>

class FilePushButton : public QPushButton {
	Q_OBJECT
public:
	enum class PathType {
		File,
		Directory
	};

public:
	FilePushButton(GlobalInfo& globalInfo, const std::string& text, const std::string& defaultPath, PathType pathType);

signals:
	void pathChanged(const std::string& path);

private slots:
	void onButtonClicked();

	void dragEnterEvent(QDragEnterEvent* event);
	void dragMoveEvent(QDragMoveEvent* event);
	void dropEvent(QDropEvent* event);

private:
	GlobalInfo& m_globalInfo;

	std::string m_defaultPath;

	PathType m_pathType;

public:
	std::string path = "";
};