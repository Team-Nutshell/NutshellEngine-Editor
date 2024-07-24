#pragma once
#include "../common/global_info.h"
#include "boolean_widget.h"
#include "file_selector_widget.h"
#include "integer_widget.h"
#include "string_widget.h"
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <string>

class OptionsFileWidget : public QWidget {
	Q_OBJECT
public:
	OptionsFileWidget(GlobalInfo& globalInfo, const std::string& optionsFilePath);

private slots:
	void onValueChanged();
	void save();

private:
	GlobalInfo& m_globalInfo;

	std::string m_optionsFilePath;

	QMenuBar* m_menuBar;
	QMenu* m_fileMenu;

	QAction* m_fileSaveAction;

public:
	StringWidget* windowTitleWidget;
	FileSelectorWidget* windowIconImageWidget;
	IntegerWidget* maxFPSWidget;
	FileSelectorWidget* firstSceneWidget;
	BooleanWidget* startProfilingWidget;
};