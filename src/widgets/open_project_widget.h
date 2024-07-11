#pragma once
#include "../common/global_info.h"
#include <QWidget>
#include <QPushButton>
#include <QFileDialog>

class OpenProjectWidget : public QWidget {
	Q_OBJECT
public:
	OpenProjectWidget(GlobalInfo& globalInfo);

signals:
	void projectDirectorySelected(const std::string&);

private slots:
	void onDirectoryPathButtonClicked();

private:
	GlobalInfo& m_globalInfo;

	QPushButton* m_directoryPathButton;
};