#pragma once
#include "../common/global_info.h"
#include "combo_box_widget.h"
#include "integer_widget.h"
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QUndoStack>
#include <QUndoCommand>
#include <string>

struct SamplerNtsp {
	std::string magFilter = "Linear";
	std::string minFilter = "Linear";
	std::string mipmapFilter = "Linear";
	std::string addressModeU = "Repeat";
	std::string addressModeV = "Repeat";
	std::string addressModeW = "Repeat";
	std::string borderColor = "FloatTransparentBlack";
	int anisotropyLevel = 0;

	bool operator==(const SamplerNtsp& rhs) {
		return (magFilter == rhs.magFilter) &&
			(minFilter == rhs.minFilter) &&
			(mipmapFilter == rhs.mipmapFilter) &&
			(addressModeU == rhs.addressModeU) &&
			(addressModeV == rhs.addressModeV) &&
			(addressModeW == rhs.addressModeW) &&
			(borderColor == rhs.borderColor) &&
			(anisotropyLevel == rhs.anisotropyLevel);
	}

	bool operator!=(const SamplerNtsp& rhs) {
		return !(*this == rhs);
	}
};

class SamplerNtspFileWidget : public QWidget {
	Q_OBJECT
public:
	SamplerNtspFileWidget(GlobalInfo& globalInfo, const std::string& samplerFilePath);

	void updateWidgets();

private slots:
	void onValueChanged();
	void save();

private:
	GlobalInfo& m_globalInfo;

	std::string m_samplerFilePath;

	QUndoStack m_undoStack;

	QMenuBar* m_menuBar;
	QMenu* m_fileMenu;
	QMenu* m_editMenu;

	QAction* m_fileSaveAction;

	QAction* m_undoAction;
	QAction* m_redoAction;

public:
	SamplerNtsp samplerNtsp;

	ComboBoxWidget* magFilterWidget;
	ComboBoxWidget* minFilterWidget;
	ComboBoxWidget* mipmapFilterWidget;
	ComboBoxWidget* addressModeUWidget;
	ComboBoxWidget* addressModeVWidget;
	ComboBoxWidget* addressModeWWidget;
	ComboBoxWidget* borderColorWidget;
	IntegerWidget* anisotropyLevelWidget;
};

class ChangeSamplerNtspFile : public QUndoCommand {
public:
	ChangeSamplerNtspFile(SamplerNtspFileWidget* samplerNtspFileWidget, SamplerNtsp newSamplerNtsp);

	void undo();
	void redo();

private:
	SamplerNtspFileWidget* m_samplerNtspFileWidget;

	SamplerNtsp m_oldSamplerNtsp;
	SamplerNtsp m_newSamplerNtsp;
};