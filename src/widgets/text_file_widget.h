#pragma once
#include "text_widget.h"
#include "../common/global_info.h"
#include <QWidget>
#include <string>

class TextFileWidget : public QWidget {
	Q_OBJECT
public:
	TextFileWidget(GlobalInfo& globalInfo);

	void setPath(const std::string& path);

	void updateWidgets();
	void save();

private slots:
	void onValueChanged();

private:
	GlobalInfo& m_globalInfo;

	std::string m_textFilePath;

public:
	std::string text;

	TextWidget* textEditWidget;
};

class ChangeTextFile : public QUndoCommand {
public:
	ChangeTextFile(GlobalInfo& globalInfo, std::string newText, const std::string& filePath);

	void undo();
	void redo();

private:
	GlobalInfo& m_globalInfo;

	TextFileWidget* m_textFileWidget;

	std::string m_filePath;
	std::string m_oldText;
	std::string m_newText;
};