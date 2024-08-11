#pragma once
#include "../common/global_info.h"
#include "combo_box_widget.h"
#include "integer_widget.h"
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <string>

class SamplerNtspFileWidget : public QWidget {
	Q_OBJECT
public:
	SamplerNtspFileWidget(GlobalInfo& globalInfo, const std::string& samplerFilePath);

private slots:
	void onValueChanged();
	void save();

private:
	GlobalInfo& m_globalInfo;

	std::string m_samplerFilePath;

	QMenuBar* m_menuBar;
	QMenu* m_fileMenu;

	QAction* m_fileSaveAction;

public:
	ComboBoxWidget* magFilterWidget;
	ComboBoxWidget* minFilterWidget;
	ComboBoxWidget* mipmapFilterWidget;
	ComboBoxWidget* addressModeUWidget;
	ComboBoxWidget* addressModeVWidget;
	ComboBoxWidget* addressModeWWidget;
	ComboBoxWidget* borderColorWidget;
	IntegerWidget* anisotropyLevelWidget;
};