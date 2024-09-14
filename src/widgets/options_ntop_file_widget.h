#pragma once
#include "../common/global_info.h"
#include "boolean_widget.h"
#include "file_selector_widget.h"
#include "integer_widget.h"
#include "string_widget.h"
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
	OptionsNtopFileWidget(GlobalInfo& globalInfo);

	void setPath(const std::string& path);

	void updateWidgets();
	void save();

private slots:
	void onValueChanged();

private:
	GlobalInfo& m_globalInfo;

	std::string m_optionsFilePath;

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
	ChangeOptionsNtopFile(GlobalInfo& globalInfo, OptionsNtop newOptionsNtop, const std::string& filePath);

	void undo();
	void redo();

private:
	GlobalInfo& m_globalInfo;

	OptionsNtopFileWidget* m_optionsNtopFileWidget;

	std::string m_filePath;
	OptionsNtop m_oldOptionsNtop;
	OptionsNtop m_newOptionsNtop;
};