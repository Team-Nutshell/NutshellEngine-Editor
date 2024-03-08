#pragma once
#include "../common/common.h"
#include <QWidget>
#include <QPushButton>
#include <QFileDialog>
#include <memory>

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

	std::unique_ptr<QPushButton> m_directoryPathButton;
};