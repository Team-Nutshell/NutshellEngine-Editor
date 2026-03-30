#pragma once
#include "code_editor_widget.h"
#include "../common/global_info.h"
#include <QWidget>
#include <QPushButton>
#include <string>

class ShaderFileWidget : public QWidget {
	Q_OBJECT
public:
	ShaderFileWidget(GlobalInfo& globalInfo);

	void setPath(const std::string& path);
	std::string getPath();

	void updateWidgets();
	void save();

private slots:
	void onValueChanged();
	void onCompileButtonClicked();

private:
	GlobalInfo& m_globalInfo;

	std::string m_shaderFilePath;

public:
	std::string text;

	CodeEditorWidget* codeEditorWidget;
	QPushButton* compileButton;
};

class ChangeShaderFile : public QUndoCommand {
public:
	ChangeShaderFile(GlobalInfo& globalInfo, std::string newText, const std::string& filePath);

	void undo();
	void redo();

private:
	GlobalInfo& m_globalInfo;

	ShaderFileWidget* m_shaderFileWidget;

	std::string m_filePath;
	std::string m_oldText;
	std::string m_newText;
};