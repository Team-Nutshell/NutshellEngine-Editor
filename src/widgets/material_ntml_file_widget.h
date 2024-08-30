#pragma once
#include "../common/global_info.h"
#include "file_selector_widget.h"
#include "scalar_widget.h"
#include "color_picker_widget.h"
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <string>

class MaterialNtmlFileWidget : public QWidget {
	Q_OBJECT
public:
	MaterialNtmlFileWidget(GlobalInfo& globalInfo, const std::string& materialFilePath);

private slots:
	void onValueChanged();
	void save();

private:
	GlobalInfo& m_globalInfo;

	std::string m_materialFilePath;

	QMenuBar* m_menuBar;
	QMenu* m_fileMenu;

	QAction* m_fileSaveAction;

public:
	FileSelectorWidget* diffuseTextureImageWidget;
	FileSelectorWidget* diffuseTextureImageSamplerWidget;
	ColorPickerWidget* diffuseColorWidget;
	ScalarWidget* opacityValueWidget;
	FileSelectorWidget* normalTextureImageWidget;
	FileSelectorWidget* normalTextureImageSamplerWidget;
	FileSelectorWidget* metalnessTextureImageWidget;
	FileSelectorWidget* metalnessTextureImageSamplerWidget;
	ScalarWidget* metalnessValueWidget;
	FileSelectorWidget* roughnessTextureImageWidget;
	FileSelectorWidget* roughnessTextureImageSamplerWidget;
	ScalarWidget* roughnessValueWidget;
	FileSelectorWidget* occlusionTextureImageWidget;
	FileSelectorWidget* occlusionTextureImageSamplerWidget;
	ScalarWidget* occlusionValueWidget;
	FileSelectorWidget* emissiveTextureImageWidget;
	FileSelectorWidget* emissiveTextureImageSamplerWidget;
	ColorPickerWidget* emissiveColorWidget;
	ScalarWidget* emissiveFactorWidget;
	ScalarWidget* alphaCutoffWidget;
	ScalarWidget* indexOfRefractionWidget;
};