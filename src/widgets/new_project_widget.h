#pragma once
#include "../common/common.h"
#include "new_project_directory_path_widget.h"
#include "new_project_name_widget.h"
#include <QWidget>
#include <QLabel>
#include <QPushButton>

class NewProjectWidget : public QWidget {
	Q_OBJECT
public:
	NewProjectWidget(GlobalInfo& globalInfo);

signals:
	void newProjectButtonClicked(const std::string&, const std::string&);

private slots:
	void onCreateNewProjectButtonClicked();
	void onDirectorySelected(const std::string& directoryPath);
	void onTextChanged(const std::string& text);

private:
	GlobalInfo& m_globalInfo;

	std::unique_ptr<NewProjectDirectoryPathWidget> m_newProjectDirectoryPathWidget;
	std::unique_ptr<NewProjectNameWidget> m_newProjectNameWidget;
	std::unique_ptr<QLabel> m_projectPathLabel;
	std::unique_ptr<QPushButton> m_createNewProjectButton;

	std::string m_projectDirectoryPath = "";
	std::string m_projectName = "";
};