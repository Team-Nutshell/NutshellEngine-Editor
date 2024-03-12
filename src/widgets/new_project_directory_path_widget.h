#pragma once
#include "../common/common.h"
#include <QWidget>
#include <QPushButton>

class NewProjectDirectoryPathWidget : public QWidget {
	Q_OBJECT
public:
	NewProjectDirectoryPathWidget(GlobalInfo& globalInfo);

signals:
	void newProjectDirectorySelected(const std::string& newProjectDirectory);

private slots:
	void onDirectoryPathButtonClicked();

private:
	GlobalInfo& m_globalInfo;

	QPushButton* m_directoryPathButton;
};