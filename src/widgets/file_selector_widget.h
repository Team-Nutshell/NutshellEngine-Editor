#pragma once
#include "../common/global_info.h"
#include "file_push_button.h"
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QMouseEvent>
#include <string>

class FileSelectorWidget : public QWidget {
	Q_OBJECT
public:
	FileSelectorWidget(GlobalInfo& globalInfo, const std::string& labelText, const std::string& noFileText, const std::string& defaultPath);

	void setPath(const std::string& path);
	const std::string& getPath();

signals:
	void fileSelected(const std::string&);

private slots:
	void onPathChanged(const std::string& path);
	void onResetFilePathClicked();

	void mousePressEvent(QMouseEvent* event);

private:
	GlobalInfo& m_globalInfo;

	std::string m_path = "";
	std::string m_noFileText;

public:
	QLabel* filePathLabel;
	FilePushButton* filePathButton;
	QPushButton* resetFilePathButton;
};
