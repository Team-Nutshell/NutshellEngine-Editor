#pragma once
#include "../common/global_info.h"
#include "file_push_button.h"
#include <QWidget>

class NewProjectDirectoryPathWidget : public QWidget {
	Q_OBJECT
public:
	NewProjectDirectoryPathWidget(GlobalInfo& globalInfo);

signals:
	void newProjectDirectorySelected(const std::string& newProjectDirectory);

private slots:
	void onPathChanged(const std::string& path);

private:
	GlobalInfo& m_globalInfo;

	FilePushButton* m_directoryPathButton;
};