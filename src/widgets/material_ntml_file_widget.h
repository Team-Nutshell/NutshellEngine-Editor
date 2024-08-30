#pragma once
#include "../common/global_info.h"
#include "file_selector_widget.h"
#include "scalar_widget.h"
#include "color_picker_widget.h"
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QUndoStack>
#include <QUndoCommand>
#include <string>

struct MaterialNtml {
	std::string diffuseTextureImagePath = "";
	std::string diffuseTextureImageSamplerPath = "";
	nml::vec3 diffuseColor = nml::vec3(0.5f, 0.5f, 0.5f);
	float opacity = 1.0f;
	std::string normalTextureImagePath = "";
	std::string normalTextureImageSamplerPath = "";
	std::string metalnessTextureImagePath = "";
	std::string metalnessTextureImageSamplerPath = "";
	float metalnessValue = 0.5f;
	std::string roughnessTextureImagePath = "";
	std::string roughnessTextureImageSamplerPath = "";
	float roughnessValue = 0.5f;
	std::string occlusionTextureImagePath = "";
	std::string occlusionTextureImageSamplerPath = "";
	float occlusionValue = 1.0f;
	std::string emissiveTextureImagePath = "";
	std::string emissiveTextureImageSamplerPath = "";
	nml::vec3 emissiveColor = nml::vec3(1.0f, 1.0f, 1.0f);
	float emissiveFactor = 1.0f;
	float alphaCutoff = 0.0f;
	float indexOfRefraction = 0.0f;

	bool operator==(const MaterialNtml& rhs) {
		return (diffuseTextureImagePath == rhs.diffuseTextureImagePath) &&
			(diffuseTextureImageSamplerPath == rhs.diffuseTextureImageSamplerPath) &&
			(diffuseColor == rhs.diffuseColor) &&
			(opacity == rhs.opacity) &&
			(normalTextureImagePath == rhs.normalTextureImagePath) &&
			(normalTextureImageSamplerPath == rhs.normalTextureImageSamplerPath) &&
			(metalnessTextureImagePath == rhs.metalnessTextureImagePath) &&
			(metalnessTextureImageSamplerPath == rhs.metalnessTextureImageSamplerPath) &&
			(metalnessValue == rhs.metalnessValue) &&
			(roughnessTextureImagePath == rhs.roughnessTextureImagePath) &&
			(roughnessTextureImageSamplerPath == rhs.roughnessTextureImageSamplerPath) &&
			(roughnessValue == rhs.roughnessValue) &&
			(occlusionTextureImagePath == rhs.occlusionTextureImagePath) &&
			(occlusionTextureImageSamplerPath == rhs.occlusionTextureImageSamplerPath) &&
			(occlusionValue == rhs.occlusionValue) &&
			(emissiveTextureImagePath == rhs.emissiveTextureImagePath) &&
			(emissiveTextureImageSamplerPath == rhs.emissiveTextureImageSamplerPath) &&
			(emissiveColor == rhs.emissiveColor) &&
			(emissiveFactor == rhs.emissiveFactor) &&
			(alphaCutoff == rhs.alphaCutoff) &&
			(indexOfRefraction == rhs.indexOfRefraction);
	}

	bool operator!=(const MaterialNtml& rhs) {
		return !(*this == rhs);
	}
};

class MaterialNtmlFileWidget : public QWidget {
	Q_OBJECT
public:
	MaterialNtmlFileWidget(GlobalInfo& globalInfo, const std::string& materialFilePath);

	void updateWidgets();

private slots:
	void onValueChanged();
	void save();

private:
	GlobalInfo& m_globalInfo;

	std::string m_materialFilePath;

	QUndoStack m_undoStack;

	QMenuBar* m_menuBar;
	QMenu* m_fileMenu;
	QMenu* m_editMenu;

	QAction* m_fileSaveAction;

	QAction* m_undoAction;
	QAction* m_redoAction;

public:
	MaterialNtml materialNtml;

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

class ChangeMaterialNtmlFile : public QUndoCommand {
public:
	ChangeMaterialNtmlFile(MaterialNtmlFileWidget* materialNtmlFileWidget, MaterialNtml newMaterialNtml);

	void undo();
	void redo();

private:
	MaterialNtmlFileWidget* m_materialNtmlFileWidget;

	MaterialNtml m_oldMaterialNtml;
	MaterialNtml m_newMaterialNtml;
};