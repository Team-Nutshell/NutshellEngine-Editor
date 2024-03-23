#pragma once
#include "../common/common.h"
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <string>

class FileSelectorWidget : public QWidget {
	Q_OBJECT
public:
	FileSelectorWidget(GlobalInfo& globalInfo, const std::string& noFileText, const std::string& buttonText, const std::string& defaultPath);

signals:
	void fileSelected(const std::string&);

private slots:
	void onFilePathButtonClicked();

private:
	GlobalInfo& m_globalInfo;

	std::string m_defaultPath = "";

public:
	QLabel* filePathLabel;
	QPushButton* filePathButton;

	std::string filePath = "";
};