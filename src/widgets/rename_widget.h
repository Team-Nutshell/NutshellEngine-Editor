#pragma once
#include "../common/global_info.h"
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QKeyEvent>
#include <regex>

class RenameWidget : public QWidget {
	Q_OBJECT
public:
	RenameWidget(GlobalInfo& globalInfo, const std::string& directory, const std::string& filename);

signals:
	void renameFileSignal(const std::string&, const std::string&);

private slots:
	void onTextChanged();
	void onButtonClicked();

	void keyPressEvent(QKeyEvent* event);

private:
	GlobalInfo& m_globalInfo;

	std::string m_directory = "";
	std::string m_filename = "";
	std::string m_newFilename = "";

public:
	QLineEdit* filenameEdit;
	QPushButton* applyButton;
};