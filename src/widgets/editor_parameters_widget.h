#pragma once
#include "../common/global_info.h"
#include "file_selector_widget.h"
#include "string_widget.h"
#include "key_select_widget.h"
#include "color_picker_widget.h"
#include "scalar_widget.h"
#include <QTabWidget>
#include <QWidget>
#include <QMenuBar>
#include <QMenu>
#include <QAction>

class EditorParametersWidget : public QTabWidget {
	Q_OBJECT
public:
	EditorParametersWidget(GlobalInfo& globalInfo);

private slots:
	void onKeyChanged(const std::string& key);
	void onScalarChanged(float value);
	void onCMakePathChanged(const std::string& cMakePath);
	void onCodeEditorCommandChanged(const std::string& codeEditorCommandChanged);
	void onCurrentEntityOutlineColorChanged(const nml::vec4& currentEntityOutlineColor);
	void onOtherEntitiesOutlineColorChanged(const nml::vec4& otherEntitiesOutlineColor);

private:
	void save();

private:
	GlobalInfo& m_globalInfo;

public:
	QWidget* rendererParametersTab;
	KeySelectWidget* cameraForwardKeySelect;
	KeySelectWidget* cameraBackwardKeySelect;
	KeySelectWidget* cameraLeftKeySelect;
	KeySelectWidget* cameraRightKeySelect;
	KeySelectWidget* cameraUpKeySelect;
	KeySelectWidget* cameraDownKeySelect;
	KeySelectWidget* switchCameraProjectionKeySelect;
	KeySelectWidget* resetCameraKeySelect;
	KeySelectWidget* orthographicCameraToXMKeySelect;
	KeySelectWidget* orthographicCameraToXPKeySelect;
	KeySelectWidget* orthographicCameraToYMKeySelect;
	KeySelectWidget* orthographicCameraToYPKeySelect;
	KeySelectWidget* orthographicCameraToZMKeySelect;
	KeySelectWidget* orthographicCameraToZPKeySelect;
	KeySelectWidget* translateEntityKeySelect;
	KeySelectWidget* rotateEntityKeySelect;
	KeySelectWidget* scaleEntityKeySelect;
	KeySelectWidget* toggleCurrentEntityVisibilityKeySelect;
	KeySelectWidget* toggleGridVisibilityKeySelect;
	KeySelectWidget* toggleBackfaceCullingKeySelect;
	KeySelectWidget* toggleCamerasVisibilityKeySelect;
	KeySelectWidget* toggleLightingKeySelect;
	KeySelectWidget* toggleCollidersVisibilityKeySelect;
	KeySelectWidget* multiSelectionKeySelect;
	ScalarWidget* cameraSpeedWidget;
	ScalarWidget* cameraSensitivityWidget;
	ColorPickerWidget* currentEntityOutlineColorWidget;
	ColorPickerWidget* otherEntitiesOutlineColorWidget;

	QWidget* buildParametersTab;
	StringWidget* cMakePathWidget;

	QWidget* codeParametersTab;
	StringWidget* codeEditorCommandWidget;
};