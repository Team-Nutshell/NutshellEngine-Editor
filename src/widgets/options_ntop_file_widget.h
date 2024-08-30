#pragma once
#include "../common/global_info.h"
#include "boolean_widget.h"
#include "file_selector_widget.h"
#include "integer_widget.h"
#include "string_widget.h"
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QUndoStack>
#include <QUndoCommand>
#include <string>

struct OptionsNtop {
	std::string windowTitle = "";
	std::string windowIconImagePath = "";
	int maxFPS = 0;
	std::string firstScenePath = "";
	bool startProfiling = false;

	bool operator==(const OptionsNtop& rhs) {
		return (windowTitle == rhs.windowTitle) &&
			(windowIconImagePath == rhs.windowIconImagePath) &&
			(maxFPS == rhs.maxFPS) &&
			(firstScenePath == rhs.firstScenePath) &&
			(startProfiling == rhs.startProfiling);
	}

	bool operator!=(const OptionsNtop& rhs) {
		return !(*this == rhs);
	}
};

class OptionsNtopFileWidget : public QWidget {
	Q_OBJECT
public:
	OptionsNtopFileWidget(GlobalInfo& globalInfo, const std::string& optionsFilePath);

	void updateWidgets();

private slots:
	void onValueChanged();
	void save();

private:
	GlobalInfo& m_globalInfo;

	std::string m_optionsFilePath;

	QUndoStack m_undoStack;

	QMenuBar* m_menuBar;
	QMenu* m_fileMenu;
	QMenu* m_editMenu;

	QAction* m_fileSaveAction;

	QAction* m_undoAction;
	QAction* m_redoAction;

public:
	OptionsNtop optionsNtop;

	StringWidget* windowTitleWidget;
	FileSelectorWidget* windowIconImageWidget;
	IntegerWidget* maxFPSWidget;
	FileSelectorWidget* firstSceneWidget;
	BooleanWidget* startProfilingWidget;
};

class ChangeOptionsNtopFile : public QUndoCommand {
public:
	ChangeOptionsNtopFile(OptionsNtopFileWidget* optionsNtopFileWidget, OptionsNtop newOptionsNtop);

	void undo();
	void redo();

private:
	OptionsNtopFileWidget* m_optionsNtopFileWidget;

	OptionsNtop m_oldOptionsNtop;
	OptionsNtop m_newOptionsNtop;
};