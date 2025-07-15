#pragma once
#include "../common/global_info.h"
#include "file_selector_widget.h"
#include "string_widget.h"
#include "key_select_widget.h"
#include "boolean_widget.h"
#include "color_picker_widget.h"
#include "scalar_widget.h"
#include "vector3_widget.h"
#include <QTabWidget>
#include <QWidget>
#include <QCloseEvent>

class EditorParametersWidget : public QTabWidget {
	Q_OBJECT
public:
	EditorParametersWidget(GlobalInfo& globalInfo);

private:
	void save();
	
	bool authorizedKey(Qt::Key key);

private slots:
	void onKeyChanged(const std::string& key);
	void onBooleanChanged(bool value);
	void onScalarChanged(float value);
	void onStringChanged(const std::string& text);
	void onVector3Changed(const nml::vec3& value);
	void onColorChanged(const nml::vec3& color);
	void closeEvent(QCloseEvent* event);

signals:
	void closeWindow();

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
	KeySelectWidget* cameraGoToEntityKeySelect;
	KeySelectWidget* translateEntityKeySelect;
	KeySelectWidget* rotateEntityKeySelect;
	KeySelectWidget* scaleEntityKeySelect;
	KeySelectWidget* toggleCurrentEntityVisibilityKeySelect;
	KeySelectWidget* toggleGridVisibilityKeySelect;
	KeySelectWidget* toggleBackfaceCullingKeySelect;
	KeySelectWidget* toggleCamerasVisibilityKeySelect;
	KeySelectWidget* toggleLightingKeySelect;
	KeySelectWidget* toggleCollidersVisibilityKeySelect;
	ScalarWidget* cameraNearPlaneWidget;
	ScalarWidget* cameraFarPlaneWidget;
	ScalarWidget* perspectiveCameraSpeedWidget;
	ScalarWidget* orthographicCameraSpeedWidget;
	ScalarWidget* cameraSensitivityWidget;
	ScalarWidget* gridScaleWidget;
	BooleanWidget* maintainGizmoSizeWidget;
	ScalarWidget* gizmoSizeWidget;
	Vector3Widget* gizmoTranslationStepWidget;
	Vector3Widget* gizmoRotationStepWidget;
	Vector3Widget* gizmoScaleStepWidget;
	ColorPickerWidget* currentEntityOutlineColorWidget;
	ColorPickerWidget* otherEntitiesOutlineColorWidget;

	QWidget* buildParametersTab;
	StringWidget* cMakePathWidget;

	QWidget* codeParametersTab;
	StringWidget* codeEditorCommandWidget;
};