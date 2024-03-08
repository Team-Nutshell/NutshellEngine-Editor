#pragma once
#include "../common/common.h"
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <string>
#include <memory>

class FileSelectorWidget : public QWidget {
	Q_OBJECT
public:
	FileSelectorWidget(GlobalInfo& globalInfo, const std::string& noFileText, const std::string& buttonText);

signals:
	void fileSelected(const std::string&);

private slots:
	void onFilePathButtonClicked();

private:
	GlobalInfo& m_globalInfo;

	std::string m_filePath = "";

public:
	std::unique_ptr<QLabel> filePathLabel;
	std::unique_ptr<QPushButton> filePathButton;
};