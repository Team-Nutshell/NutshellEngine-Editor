#pragma once
#include "../common/global_info.h"
#include "file_push_button.h"
#include <QWidget>
#include <QFileDialog>

class OpenProjectWidget : public QWidget {
	Q_OBJECT
public:
	OpenProjectWidget(GlobalInfo& globalInfo);

signals:
	void projectDirectorySelected(const std::string&);

private slots:
	void onPathChanged(const std::string& path);

private:
	GlobalInfo& m_globalInfo;

	FilePushButton* m_directoryPathButton;
};