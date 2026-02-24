#include "editor_parameters_widget.h"
#include "main_window.h"
#include "separator_line.h"
#include <QGridLayout>
#include <QVBoxLayout>
#include <fstream>

EditorParametersWidget::EditorParametersWidget(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	resize(1000, 525);
	setWindowTitle("NutshellEngine - " + QString::fromStdString(m_globalInfo.localization.getString("editor_parameters")));
	setWindowIcon(QIcon("assets/icon.png"));
	setAttribute(Qt::WA_DeleteOnClose);

	QMargins contentMargins = contentsMargins();
	contentMargins.setTop(contentMargins.top() + 10);
	setContentsMargins(contentMargins);

	rendererParametersTab = new QTabWidget();
	QVBoxLayout* rendererVerticalLayout = new QVBoxLayout();
	rendererParametersTab->setLayout(rendererVerticalLayout);
	rendererParametersTab->layout()->setAlignment(Qt::AlignmentFlag::AlignTop);

	QWidget* rendererKeyGridLayoutWidget = new QWidget();
	QGridLayout* rendererKeyGridLayout = new QGridLayout();
	rendererKeyGridLayout->setContentsMargins(0, 0, 0, 0);
	rendererKeyGridLayoutWidget->setLayout(rendererKeyGridLayout);
	rendererVerticalLayout->addWidget(rendererKeyGridLayoutWidget);

	QKeySequence cameraForwardKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.cameraForwardKey);
	cameraForwardKeySelect = new KeySelectWidget(m_globalInfo, m_globalInfo.localization.getString("editor_parameters_camera_forward"), cameraForwardKeySequence.toString().toStdString());
	cameraForwardKeySelect->layout()->setAlignment(cameraForwardKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	rendererKeyGridLayout->addWidget(cameraForwardKeySelect, 0, 0);

	QKeySequence cameraBackwardKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.cameraBackwardKey);
	cameraBackwardKeySelect = new KeySelectWidget(m_globalInfo, m_globalInfo.localization.getString("editor_parameters_camera_backward"), cameraBackwardKeySequence.toString().toStdString());
	cameraBackwardKeySelect->layout()->setAlignment(cameraBackwardKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	rendererKeyGridLayout->addWidget(cameraBackwardKeySelect, 1, 0);

	QKeySequence cameraLeftKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.cameraLeftKey);
	cameraLeftKeySelect = new KeySelectWidget(m_globalInfo, m_globalInfo.localization.getString("editor_parameters_camera_left"), cameraLeftKeySequence.toString().toStdString());
	cameraLeftKeySelect->layout()->setAlignment(cameraLeftKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	rendererKeyGridLayout->addWidget(cameraLeftKeySelect, 2, 0);

	QKeySequence cameraRightKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.cameraRightKey);
	cameraRightKeySelect = new KeySelectWidget(m_globalInfo, m_globalInfo.localization.getString("editor_parameters_camera_right"), cameraRightKeySequence.toString().toStdString());
	cameraRightKeySelect->layout()->setAlignment(cameraRightKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	rendererKeyGridLayout->addWidget(cameraRightKeySelect, 3, 0);

	QKeySequence cameraUpKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.cameraUpKey);
	cameraUpKeySelect = new KeySelectWidget(m_globalInfo, m_globalInfo.localization.getString("editor_parameters_camera_up"), cameraUpKeySequence.toString().toStdString());
	cameraUpKeySelect->layout()->setAlignment(cameraUpKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	rendererKeyGridLayout->addWidget(cameraUpKeySelect, 4, 0);

	QKeySequence cameraDownKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.cameraDownKey);
	cameraDownKeySelect = new KeySelectWidget(m_globalInfo, m_globalInfo.localization.getString("editor_parameters_camera_down"), cameraDownKeySequence.toString().toStdString());
	cameraDownKeySelect->layout()->setAlignment(cameraDownKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	rendererKeyGridLayout->addWidget(cameraDownKeySelect, 5, 0);

	QKeySequence switchCameraProjectionKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.switchCameraProjectionKey);
	switchCameraProjectionKeySelect = new KeySelectWidget(m_globalInfo, m_globalInfo.localization.getString("editor_parameters_switch_camera_projection"), switchCameraProjectionKeySequence.toString().toStdString());
	switchCameraProjectionKeySelect->layout()->setAlignment(switchCameraProjectionKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	rendererKeyGridLayout->addWidget(switchCameraProjectionKeySelect, 6, 0);

	QKeySequence resetCameraKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.resetCameraKey);
	resetCameraKeySelect = new KeySelectWidget(m_globalInfo, m_globalInfo.localization.getString("editor_parameters_reset_camera"), resetCameraKeySequence.toString().toStdString());
	resetCameraKeySelect->layout()->setAlignment(resetCameraKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	rendererKeyGridLayout->addWidget(resetCameraKeySelect, 7, 0);

	QKeySequence orthographicCameraToXMKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.orthographicCameraToXMKey);
	orthographicCameraToXMKeySelect = new KeySelectWidget(m_globalInfo, m_globalInfo.localization.getString("editor_parameters_orthographic_camera_xm"), orthographicCameraToXMKeySequence.toString().toStdString());
	orthographicCameraToXMKeySelect->layout()->setAlignment(orthographicCameraToXMKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	rendererKeyGridLayout->addWidget(orthographicCameraToXMKeySelect, 0, 1);

	QKeySequence orthographicCameraToXPKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.orthographicCameraToXPKey);
	orthographicCameraToXPKeySelect = new KeySelectWidget(m_globalInfo, m_globalInfo.localization.getString("editor_parameters_orthographic_camera_xp"), orthographicCameraToXPKeySequence.toString().toStdString());
	orthographicCameraToXPKeySelect->layout()->setAlignment(orthographicCameraToXPKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	rendererKeyGridLayout->addWidget(orthographicCameraToXPKeySelect, 1, 1);

	QKeySequence orthographicCameraToYMKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.orthographicCameraToYMKey);
	orthographicCameraToYMKeySelect = new KeySelectWidget(m_globalInfo, m_globalInfo.localization.getString("editor_parameters_orthographic_camera_ym"), orthographicCameraToYMKeySequence.toString().toStdString());
	orthographicCameraToYMKeySelect->layout()->setAlignment(orthographicCameraToYMKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	rendererKeyGridLayout->addWidget(orthographicCameraToYMKeySelect, 2, 1);

	QKeySequence orthographicCameraToYPKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.orthographicCameraToYPKey);
	orthographicCameraToYPKeySelect = new KeySelectWidget(m_globalInfo, m_globalInfo.localization.getString("editor_parameters_orthographic_camera_yp"), orthographicCameraToYPKeySequence.toString().toStdString());
	orthographicCameraToYPKeySelect->layout()->setAlignment(orthographicCameraToYPKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	rendererKeyGridLayout->addWidget(orthographicCameraToYPKeySelect, 3, 1);

	QKeySequence orthographicCameraToZMKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.orthographicCameraToZMKey);
	orthographicCameraToZMKeySelect = new KeySelectWidget(m_globalInfo, m_globalInfo.localization.getString("editor_parameters_orthographic_camera_zm"), orthographicCameraToZMKeySequence.toString().toStdString());
	orthographicCameraToZMKeySelect->layout()->setAlignment(orthographicCameraToZMKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	rendererKeyGridLayout->addWidget(orthographicCameraToZMKeySelect, 4, 1);

	QKeySequence orthographicCameraToZPKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.orthographicCameraToZPKey);
	orthographicCameraToZPKeySelect = new KeySelectWidget(m_globalInfo, m_globalInfo.localization.getString("editor_parameters_orthographic_camera_zp"), orthographicCameraToZPKeySequence.toString().toStdString());
	orthographicCameraToZPKeySelect->layout()->setAlignment(orthographicCameraToZPKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	rendererKeyGridLayout->addWidget(orthographicCameraToZPKeySelect, 5, 1);

	QKeySequence cameraGoToEntityKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.cameraGoToEntityKey);
	cameraGoToEntityKeySelect = new KeySelectWidget(m_globalInfo, m_globalInfo.localization.getString("editor_parameters_camera_go_to_entity"), cameraGoToEntityKeySequence.toString().toStdString());
	cameraGoToEntityKeySelect->layout()->setAlignment(cameraGoToEntityKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	rendererKeyGridLayout->addWidget(cameraGoToEntityKeySelect, 6, 1);

	QKeySequence translateEntityKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.translateEntityKey);
	translateEntityKeySelect = new KeySelectWidget(m_globalInfo, m_globalInfo.localization.getString("editor_parameters_translate_entity"), translateEntityKeySequence.toString().toStdString());
	translateEntityKeySelect->layout()->setAlignment(translateEntityKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	rendererKeyGridLayout->addWidget(translateEntityKeySelect, 7, 1);

	QKeySequence rotateEntityKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.rotateEntityKey);
	rotateEntityKeySelect = new KeySelectWidget(m_globalInfo, m_globalInfo.localization.getString("editor_parameters_rotate_entity"), rotateEntityKeySequence.toString().toStdString());
	rotateEntityKeySelect->layout()->setAlignment(rotateEntityKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	rendererKeyGridLayout->addWidget(rotateEntityKeySelect, 0, 2);

	QKeySequence scaleEntityKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.scaleEntityKey);
	scaleEntityKeySelect = new KeySelectWidget(m_globalInfo, m_globalInfo.localization.getString("editor_parameters_scale_entity"), scaleEntityKeySequence.toString().toStdString());
	scaleEntityKeySelect->layout()->setAlignment(scaleEntityKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	rendererKeyGridLayout->addWidget(scaleEntityKeySelect, 1, 2);

	QKeySequence toggleCurrentEntityVisibilityKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.toggleCurrentEntityVisibilityKey);
	toggleCurrentEntityVisibilityKeySelect = new KeySelectWidget(m_globalInfo, m_globalInfo.localization.getString("editor_parameters_toggle_current_entity_visibility"), toggleCurrentEntityVisibilityKeySequence.toString().toStdString());
	toggleCurrentEntityVisibilityKeySelect->layout()->setAlignment(toggleCurrentEntityVisibilityKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	rendererKeyGridLayout->addWidget(toggleCurrentEntityVisibilityKeySelect, 2, 2);

	QKeySequence toggleGridVisibilityKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.toggleGridVisibilityKey);
	toggleGridVisibilityKeySelect = new KeySelectWidget(m_globalInfo, m_globalInfo.localization.getString("editor_parameters_toggle_grid_visibility"), toggleGridVisibilityKeySequence.toString().toStdString());
	toggleGridVisibilityKeySelect->layout()->setAlignment(toggleGridVisibilityKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	rendererKeyGridLayout->addWidget(toggleGridVisibilityKeySelect, 3, 2);

	QKeySequence toggleBackfaceCullingKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.toggleBackfaceCullingKey);
	toggleBackfaceCullingKeySelect = new KeySelectWidget(m_globalInfo, m_globalInfo.localization.getString("editor_parameters_toggle_backface_culling"), toggleBackfaceCullingKeySequence.toString().toStdString());
	toggleBackfaceCullingKeySelect->layout()->setAlignment(toggleBackfaceCullingKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	rendererKeyGridLayout->addWidget(toggleBackfaceCullingKeySelect, 4, 2);

	QKeySequence toggleCamerasVisibilityKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.toggleCamerasVisibilityKey);
	toggleCamerasVisibilityKeySelect = new KeySelectWidget(m_globalInfo, m_globalInfo.localization.getString("editor_parameters_toggle_cameras_visibility"), toggleCamerasVisibilityKeySequence.toString().toStdString());
	toggleCamerasVisibilityKeySelect->layout()->setAlignment(toggleCamerasVisibilityKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	rendererKeyGridLayout->addWidget(toggleCamerasVisibilityKeySelect, 5, 2);

	QKeySequence toggleLightingKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.toggleLightingKey);
	toggleLightingKeySelect = new KeySelectWidget(m_globalInfo, m_globalInfo.localization.getString("editor_parameters_toggle_lighting"), toggleLightingKeySequence.toString().toStdString());
	toggleLightingKeySelect->layout()->setAlignment(toggleLightingKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	rendererKeyGridLayout->addWidget(toggleLightingKeySelect, 6, 2);

	QKeySequence toggleCollidersVisibilityKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.toggleCollidersVisibilityKey);
	toggleCollidersVisibilityKeySelect = new KeySelectWidget(m_globalInfo, m_globalInfo.localization.getString("editor_parameters_toggle_colliders_visibility"), toggleCollidersVisibilityKeySequence.toString().toStdString());
	toggleCollidersVisibilityKeySelect->layout()->setAlignment(toggleCollidersVisibilityKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	rendererKeyGridLayout->addWidget(toggleCollidersVisibilityKeySelect, 7, 2);

	rendererVerticalLayout->addWidget(new SeparatorLine());

	QWidget* rendererCameraGridLayoutWidget = new QWidget();
	QGridLayout* rendererCameraGridLayout = new QGridLayout();
	rendererCameraGridLayout->setContentsMargins(0, 0, 0, 0);
	rendererCameraGridLayoutWidget->setLayout(rendererCameraGridLayout);
	rendererVerticalLayout->addWidget(rendererCameraGridLayoutWidget);

	cameraNearPlaneWidget = new ScalarWidget(m_globalInfo, m_globalInfo.localization.getString("editor_parameters_camera_near_plane"));
	cameraNearPlaneWidget->valueLineEdit->setText(QString::number(m_globalInfo.editorParameters.renderer.cameraNearPlane, 'g', 7));
	cameraNearPlaneWidget->layout()->setAlignment(cameraNearPlaneWidget->nameLabel, Qt::AlignmentFlag::AlignRight);
	cameraNearPlaneWidget->layout()->setAlignment(cameraNearPlaneWidget->valueLineEdit, Qt::AlignmentFlag::AlignLeft);
	rendererCameraGridLayout->addWidget(cameraNearPlaneWidget, 0, 0);

	cameraFarPlaneWidget = new ScalarWidget(m_globalInfo, m_globalInfo.localization.getString("editor_parameters_camera_far_plane"));
	cameraFarPlaneWidget->valueLineEdit->setText(QString::number(m_globalInfo.editorParameters.renderer.cameraFarPlane, 'g', 7));
	cameraFarPlaneWidget->layout()->setAlignment(cameraFarPlaneWidget->nameLabel, Qt::AlignmentFlag::AlignRight);
	cameraFarPlaneWidget->layout()->setAlignment(cameraFarPlaneWidget->valueLineEdit, Qt::AlignmentFlag::AlignLeft);
	rendererCameraGridLayout->addWidget(cameraFarPlaneWidget, 1, 0);

	perspectiveCameraSpeedWidget = new ScalarWidget(m_globalInfo, m_globalInfo.localization.getString("editor_parameters_perspective_camera_speed"));
	perspectiveCameraSpeedWidget->valueLineEdit->setText(QString::number(m_globalInfo.editorParameters.renderer.perspectiveCameraSpeed, 'g', 7));
	perspectiveCameraSpeedWidget->layout()->setAlignment(perspectiveCameraSpeedWidget->nameLabel, Qt::AlignmentFlag::AlignRight);
	perspectiveCameraSpeedWidget->layout()->setAlignment(perspectiveCameraSpeedWidget->valueLineEdit, Qt::AlignmentFlag::AlignLeft);
	rendererCameraGridLayout->addWidget(perspectiveCameraSpeedWidget, 0, 1);

	orthographicCameraSpeedWidget = new ScalarWidget(m_globalInfo, m_globalInfo.localization.getString("editor_parameters_orthographic_camera_speed"));
	orthographicCameraSpeedWidget->valueLineEdit->setText(QString::number(m_globalInfo.editorParameters.renderer.orthographicCameraSpeed, 'g', 7));
	orthographicCameraSpeedWidget->layout()->setAlignment(orthographicCameraSpeedWidget->nameLabel, Qt::AlignmentFlag::AlignRight);
	orthographicCameraSpeedWidget->layout()->setAlignment(orthographicCameraSpeedWidget->valueLineEdit, Qt::AlignmentFlag::AlignLeft);
	rendererCameraGridLayout->addWidget(orthographicCameraSpeedWidget, 1, 1);

	cameraSensitivityWidget = new ScalarWidget(m_globalInfo, m_globalInfo.localization.getString("editor_parameters_camera_sensitivity"));
	cameraSensitivityWidget->valueLineEdit->setText(QString::number(m_globalInfo.editorParameters.renderer.cameraSensitivity, 'g', 7));
	cameraSensitivityWidget->layout()->setAlignment(cameraSensitivityWidget->nameLabel, Qt::AlignmentFlag::AlignRight);
	cameraSensitivityWidget->layout()->setAlignment(cameraSensitivityWidget->valueLineEdit, Qt::AlignmentFlag::AlignLeft);
	rendererCameraGridLayout->addWidget(cameraSensitivityWidget, 2, 1);

	gridCellSizeWidget = new ScalarWidget(m_globalInfo, m_globalInfo.localization.getString("editor_parameters_grid_cell_size"));
	gridCellSizeWidget->valueLineEdit->setText(QString::number(m_globalInfo.editorParameters.renderer.gridCellSize, 'g', 7));
	gridCellSizeWidget->layout()->setAlignment(gridCellSizeWidget->nameLabel, Qt::AlignmentFlag::AlignRight);
	gridCellSizeWidget->layout()->setAlignment(gridCellSizeWidget->valueLineEdit, Qt::AlignmentFlag::AlignLeft);
	rendererCameraGridLayout->addWidget(gridCellSizeWidget, 0, 2);

	gridSubcellSizeWidget = new ScalarWidget(m_globalInfo, m_globalInfo.localization.getString("editor_parameters_grid_subcell_size"));
	gridSubcellSizeWidget->valueLineEdit->setText(QString::number(m_globalInfo.editorParameters.renderer.gridSubcellSize, 'g', 7));
	gridSubcellSizeWidget->layout()->setAlignment(gridSubcellSizeWidget->nameLabel, Qt::AlignmentFlag::AlignRight);
	gridSubcellSizeWidget->layout()->setAlignment(gridSubcellSizeWidget->valueLineEdit, Qt::AlignmentFlag::AlignLeft);
	rendererCameraGridLayout->addWidget(gridSubcellSizeWidget, 1, 2);

	rendererVerticalLayout->addWidget(new SeparatorLine());

	QWidget* rendererGizmoLayoutWidget = new QWidget();
	QVBoxLayout* rendererGizmoLayout = new QVBoxLayout();
	rendererGizmoLayout->setContentsMargins(0, 0, 0, 0);
	rendererGizmoLayoutWidget->setLayout(rendererGizmoLayout);
	rendererVerticalLayout->addWidget(rendererGizmoLayoutWidget);

	maintainGizmoSizeWidget = new BooleanWidget(m_globalInfo, m_globalInfo.localization.getString("editor_parameters_maintain_gizmo_size"));
	maintainGizmoSizeWidget->setValue(m_globalInfo.editorParameters.renderer.maintainGizmoSize);
	maintainGizmoSizeWidget->layout()->setAlignment(maintainGizmoSizeWidget->nameLabel, Qt::AlignmentFlag::AlignRight);
	maintainGizmoSizeWidget->layout()->setAlignment(maintainGizmoSizeWidget->checkBox, Qt::AlignmentFlag::AlignLeft);
	rendererGizmoLayout->addWidget(maintainGizmoSizeWidget);

	gizmoSizeWidget = new ScalarWidget(m_globalInfo, m_globalInfo.localization.getString("editor_parameters_gizmo_size"));
	gizmoSizeWidget->valueLineEdit->setText(QString::number(m_globalInfo.editorParameters.renderer.gizmoSize, 'g', 7));
	gizmoSizeWidget->layout()->setAlignment(gizmoSizeWidget->nameLabel, Qt::AlignmentFlag::AlignRight);
	gizmoSizeWidget->layout()->setAlignment(gizmoSizeWidget->valueLineEdit, Qt::AlignmentFlag::AlignLeft);
	rendererGizmoLayout->addWidget(gizmoSizeWidget);

	gizmoTranslationStepWidget = new Vector3Widget(m_globalInfo, m_globalInfo.localization.getString("editor_parameters_gizmo_translation_step"));
	gizmoTranslationStepWidget->setValue(m_globalInfo.editorParameters.renderer.gizmoTranslationStep);
	gizmoTranslationStepWidget->layout()->setAlignment(gizmoTranslationStepWidget->nameLabel, Qt::AlignmentFlag::AlignLeft);
	gizmoTranslationStepWidget->layout()->setAlignment(gizmoTranslationStepWidget->xLineEdit, Qt::AlignmentFlag::AlignLeft);
	gizmoTranslationStepWidget->layout()->setAlignment(gizmoTranslationStepWidget->yLineEdit, Qt::AlignmentFlag::AlignLeft);
	gizmoTranslationStepWidget->layout()->setAlignment(gizmoTranslationStepWidget->zLineEdit, Qt::AlignmentFlag::AlignLeft);
	rendererGizmoLayout->addWidget(gizmoTranslationStepWidget);

	gizmoRotationStepWidget = new Vector3Widget(m_globalInfo, m_globalInfo.localization.getString("editor_parameters_gizmo_rotation_step"));
	gizmoRotationStepWidget->setValue(m_globalInfo.editorParameters.renderer.gizmoRotationStep);
	gizmoRotationStepWidget->layout()->setAlignment(gizmoRotationStepWidget->nameLabel, Qt::AlignmentFlag::AlignLeft);
	gizmoRotationStepWidget->layout()->setAlignment(gizmoRotationStepWidget->xLineEdit, Qt::AlignmentFlag::AlignLeft);
	gizmoRotationStepWidget->layout()->setAlignment(gizmoRotationStepWidget->yLineEdit, Qt::AlignmentFlag::AlignLeft);
	gizmoRotationStepWidget->layout()->setAlignment(gizmoRotationStepWidget->zLineEdit, Qt::AlignmentFlag::AlignLeft);
	rendererGizmoLayout->addWidget(gizmoRotationStepWidget);

	gizmoScaleStepWidget = new Vector3Widget(m_globalInfo, m_globalInfo.localization.getString("editor_parameters_gizmo_scale_step"));
	gizmoScaleStepWidget->setValue(m_globalInfo.editorParameters.renderer.gizmoScaleStep);
	gizmoScaleStepWidget->layout()->setAlignment(gizmoScaleStepWidget->nameLabel, Qt::AlignmentFlag::AlignLeft);
	gizmoScaleStepWidget->layout()->setAlignment(gizmoScaleStepWidget->xLineEdit, Qt::AlignmentFlag::AlignLeft);
	gizmoScaleStepWidget->layout()->setAlignment(gizmoScaleStepWidget->yLineEdit, Qt::AlignmentFlag::AlignLeft);
	gizmoScaleStepWidget->layout()->setAlignment(gizmoScaleStepWidget->zLineEdit, Qt::AlignmentFlag::AlignLeft);
	rendererGizmoLayout->addWidget(gizmoScaleStepWidget);

	rendererVerticalLayout->addWidget(new SeparatorLine());

	currentEntityOutlineColorWidget = new ColorPickerWidget(m_globalInfo, m_globalInfo.localization.getString("editor_parameters_gizmo_current_entity_outline_color"), nml::vec3(m_globalInfo.editorParameters.renderer.currentEntityOutlineColor));
	currentEntityOutlineColorWidget->layout()->setAlignment(currentEntityOutlineColorWidget->nameLabel, Qt::AlignmentFlag::AlignRight);
	currentEntityOutlineColorWidget->layout()->setAlignment(currentEntityOutlineColorWidget->colorButton, Qt::AlignmentFlag::AlignLeft);
	rendererVerticalLayout->addWidget(currentEntityOutlineColorWidget);

	otherEntitiesOutlineColorWidget = new ColorPickerWidget(m_globalInfo, m_globalInfo.localization.getString("editor_parameters_gizmo_other_selected_entities_outline_color"), nml::vec3(m_globalInfo.editorParameters.renderer.otherEntitiesOutlineColor));
	otherEntitiesOutlineColorWidget->layout()->setAlignment(otherEntitiesOutlineColorWidget->nameLabel, Qt::AlignmentFlag::AlignRight);
	otherEntitiesOutlineColorWidget->layout()->setAlignment(otherEntitiesOutlineColorWidget->colorButton, Qt::AlignmentFlag::AlignLeft);
	rendererVerticalLayout->addWidget(otherEntitiesOutlineColorWidget);

	addTab(rendererParametersTab, QString::fromStdString(m_globalInfo.localization.getString("editor_parameters_renderer")));

	buildParametersTab = new QTabWidget();
	buildParametersTab->setLayout(new QVBoxLayout());
	buildParametersTab->layout()->setAlignment(Qt::AlignmentFlag::AlignTop);
	cMakePathWidget = new StringWidget(m_globalInfo, m_globalInfo.localization.getString("editor_parameters_cmake_executable"));
	cMakePathWidget->setText(m_globalInfo.editorParameters.build.cMakePath);
	cMakePathWidget->valueLineEdit->setFixedWidth(400);
	buildParametersTab->layout()->addWidget(cMakePathWidget);

	addTab(buildParametersTab, QString::fromStdString(m_globalInfo.localization.getString("editor_parameters_build")));

	codeParametersTab = new QTabWidget();
	codeParametersTab->setLayout(new QVBoxLayout());
	codeParametersTab->layout()->setAlignment(Qt::AlignmentFlag::AlignTop);
	codeEditorCommandWidget = new StringWidget(m_globalInfo, m_globalInfo.localization.getString("editor_parameters_code_editor_command"));
	codeEditorCommandWidget->setText(m_globalInfo.editorParameters.code.codeEditorCommand);
	codeEditorCommandWidget->valueLineEdit->setFixedWidth(400);
	codeParametersTab->layout()->addWidget(codeEditorCommandWidget);

	addTab(codeParametersTab, QString::fromStdString(m_globalInfo.localization.getString("editor_parameters_code")));

	connect(cameraForwardKeySelect, &KeySelectWidget::keyChanged, this, &EditorParametersWidget::onKeyChanged);
	connect(cameraBackwardKeySelect, &KeySelectWidget::keyChanged, this, &EditorParametersWidget::onKeyChanged);
	connect(cameraLeftKeySelect, &KeySelectWidget::keyChanged, this, &EditorParametersWidget::onKeyChanged);
	connect(cameraRightKeySelect, &KeySelectWidget::keyChanged, this, &EditorParametersWidget::onKeyChanged);
	connect(cameraUpKeySelect, &KeySelectWidget::keyChanged, this, &EditorParametersWidget::onKeyChanged);
	connect(cameraDownKeySelect, &KeySelectWidget::keyChanged, this, &EditorParametersWidget::onKeyChanged);
	connect(switchCameraProjectionKeySelect, &KeySelectWidget::keyChanged, this, &EditorParametersWidget::onKeyChanged);
	connect(resetCameraKeySelect, &KeySelectWidget::keyChanged, this, &EditorParametersWidget::onKeyChanged);
	connect(orthographicCameraToXMKeySelect, &KeySelectWidget::keyChanged, this, &EditorParametersWidget::onKeyChanged);
	connect(orthographicCameraToXPKeySelect, &KeySelectWidget::keyChanged, this, &EditorParametersWidget::onKeyChanged);
	connect(orthographicCameraToYMKeySelect, &KeySelectWidget::keyChanged, this, &EditorParametersWidget::onKeyChanged);
	connect(orthographicCameraToYPKeySelect, &KeySelectWidget::keyChanged, this, &EditorParametersWidget::onKeyChanged);
	connect(orthographicCameraToZMKeySelect, &KeySelectWidget::keyChanged, this, &EditorParametersWidget::onKeyChanged);
	connect(orthographicCameraToZPKeySelect, &KeySelectWidget::keyChanged, this, &EditorParametersWidget::onKeyChanged);
	connect(cameraGoToEntityKeySelect, &KeySelectWidget::keyChanged, this, &EditorParametersWidget::onKeyChanged);
	connect(translateEntityKeySelect, &KeySelectWidget::keyChanged, this, &EditorParametersWidget::onKeyChanged);
	connect(rotateEntityKeySelect, &KeySelectWidget::keyChanged, this, &EditorParametersWidget::onKeyChanged);
	connect(scaleEntityKeySelect, &KeySelectWidget::keyChanged, this, &EditorParametersWidget::onKeyChanged);
	connect(toggleCurrentEntityVisibilityKeySelect, &KeySelectWidget::keyChanged, this, &EditorParametersWidget::onKeyChanged);
	connect(toggleGridVisibilityKeySelect, &KeySelectWidget::keyChanged, this, &EditorParametersWidget::onKeyChanged);
	connect(toggleBackfaceCullingKeySelect, &KeySelectWidget::keyChanged, this, &EditorParametersWidget::onKeyChanged);
	connect(toggleCamerasVisibilityKeySelect, &KeySelectWidget::keyChanged, this, &EditorParametersWidget::onKeyChanged);
	connect(toggleLightingKeySelect, &KeySelectWidget::keyChanged, this, &EditorParametersWidget::onKeyChanged);
	connect(toggleCollidersVisibilityKeySelect, &KeySelectWidget::keyChanged, this, &EditorParametersWidget::onKeyChanged);
	connect(cameraNearPlaneWidget, &ScalarWidget::valueChanged, this, &EditorParametersWidget::onScalarChanged);
	connect(cameraFarPlaneWidget, &ScalarWidget::valueChanged, this, &EditorParametersWidget::onScalarChanged);
	connect(perspectiveCameraSpeedWidget, &ScalarWidget::valueChanged, this, &EditorParametersWidget::onScalarChanged);
	connect(orthographicCameraSpeedWidget, &ScalarWidget::valueChanged, this, &EditorParametersWidget::onScalarChanged);
	connect(cameraSensitivityWidget, &ScalarWidget::valueChanged, this, &EditorParametersWidget::onScalarChanged);
	connect(gridCellSizeWidget, &ScalarWidget::valueChanged, this, &EditorParametersWidget::onScalarChanged);
	connect(gridSubcellSizeWidget, &ScalarWidget::valueChanged, this, &EditorParametersWidget::onScalarChanged);
	connect(maintainGizmoSizeWidget, &BooleanWidget::stateChanged, this, &EditorParametersWidget::onBooleanChanged);
	connect(gizmoSizeWidget, &ScalarWidget::valueChanged, this, &EditorParametersWidget::onScalarChanged);
	connect(gizmoTranslationStepWidget, &Vector3Widget::valueChanged, this, &EditorParametersWidget::onVector3Changed);
	connect(gizmoRotationStepWidget, &Vector3Widget::valueChanged, this, &EditorParametersWidget::onVector3Changed);
	connect(gizmoScaleStepWidget, &Vector3Widget::valueChanged, this, &EditorParametersWidget::onVector3Changed);
	connect(currentEntityOutlineColorWidget, &ColorPickerWidget::colorChanged, this, &EditorParametersWidget::onColorChanged);
	connect(otherEntitiesOutlineColorWidget, &ColorPickerWidget::colorChanged, this, &EditorParametersWidget::onColorChanged);
	connect(currentEntityOutlineColorWidget, &ColorPickerWidget::temporaryColorChanged, this, &EditorParametersWidget::onTemporaryColorChanged);
	connect(otherEntitiesOutlineColorWidget, &ColorPickerWidget::temporaryColorChanged, this, &EditorParametersWidget::onTemporaryColorChanged);
	connect(cMakePathWidget, &StringWidget::valueChanged, this, &EditorParametersWidget::onStringChanged);
	connect(codeEditorCommandWidget, &StringWidget::valueChanged, this, &EditorParametersWidget::onStringChanged);
}

void EditorParametersWidget::save() {
	nlohmann::json j = m_globalInfo.editorParameters.toJson();

	std::fstream optionsFile("options.json", std::ios::out | std::ios::trunc);
	if (j.empty()) {
		optionsFile << "{\n}";
	}
	else {
		optionsFile << j.dump(1, '\t');
	}
}

bool EditorParametersWidget::authorizedKey(Qt::Key key) {
	return (key != Qt::Key_Shift) && (key != Qt::Key_Control) && (key != Qt::Key_Alt);
}

void EditorParametersWidget::onKeyChanged(const std::string& key) {
	KeySelectWidget* senderWidget = static_cast<KeySelectWidget*>(sender());
	if (senderWidget == cameraForwardKeySelect) {
		QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(key));
		if (!sequence.isEmpty() && authorizedKey(sequence[0].key())) {
			m_globalInfo.editorParameters.renderer.cameraForwardKey = sequence[0].key();
		}
		else {
			m_globalInfo.logger.addLog(LogLevel::Warning, m_globalInfo.localization.getString("log_binding_unauthorized", { m_globalInfo.localization.getString("editor_parameters_camera_forward"), key }));
			senderWidget->setKey(m_globalInfo.editorParameters.renderer.cameraForwardKey);
		}
	}
	else if (senderWidget == cameraBackwardKeySelect) {
		QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(key));
		if (!sequence.isEmpty() && authorizedKey(sequence[0].key())) {
			m_globalInfo.editorParameters.renderer.cameraBackwardKey = sequence[0].key();
		}
		else {
			m_globalInfo.logger.addLog(LogLevel::Warning, m_globalInfo.localization.getString("log_binding_unauthorized", { m_globalInfo.localization.getString("editor_parameters_camera_backward"), key }));
			senderWidget->setKey(m_globalInfo.editorParameters.renderer.cameraBackwardKey);
		}
	}
	else if (senderWidget == cameraLeftKeySelect) {
		QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(key));
		if (!sequence.isEmpty() && authorizedKey(sequence[0].key())) {
			m_globalInfo.editorParameters.renderer.cameraLeftKey = sequence[0].key();
		}
		else {
			m_globalInfo.logger.addLog(LogLevel::Warning, m_globalInfo.localization.getString("log_binding_unauthorized", { m_globalInfo.localization.getString("editor_parameters_camera_left"), key }));
			senderWidget->setKey(m_globalInfo.editorParameters.renderer.cameraLeftKey);
		}
	}
	else if (senderWidget == cameraRightKeySelect) {
		QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(key));
		if (!sequence.isEmpty() && authorizedKey(sequence[0].key())) {
			m_globalInfo.editorParameters.renderer.cameraRightKey = sequence[0].key();
		}
		else {
			m_globalInfo.logger.addLog(LogLevel::Warning, m_globalInfo.localization.getString("log_binding_unauthorized", { m_globalInfo.localization.getString("editor_parameters_camera_right"), key }));
			senderWidget->setKey(m_globalInfo.editorParameters.renderer.cameraRightKey);
		}
	}
	else if (senderWidget == cameraUpKeySelect) {
		QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(key));
		if (!sequence.isEmpty() && authorizedKey(sequence[0].key())) {
			m_globalInfo.editorParameters.renderer.cameraUpKey = sequence[0].key();
		}
		else {
			m_globalInfo.logger.addLog(LogLevel::Warning, m_globalInfo.localization.getString("log_binding_unauthorized", { m_globalInfo.localization.getString("editor_parameters_camera_up"), key }));
			senderWidget->setKey(m_globalInfo.editorParameters.renderer.cameraUpKey);
		}
	}
	else if (senderWidget == cameraDownKeySelect) {
		QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(key));
		if (!sequence.isEmpty() && authorizedKey(sequence[0].key())) {
			m_globalInfo.editorParameters.renderer.cameraDownKey = sequence[0].key();
		}
		else {
			m_globalInfo.logger.addLog(LogLevel::Warning, m_globalInfo.localization.getString("log_binding_unauthorized", { m_globalInfo.localization.getString("editor_parameters_camera_down"), key }));
			senderWidget->setKey(m_globalInfo.editorParameters.renderer.cameraDownKey);
		}
	}
	else if (senderWidget == switchCameraProjectionKeySelect) {
		QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(key));
		if (!sequence.isEmpty() && authorizedKey(sequence[0].key())) {
			m_globalInfo.editorParameters.renderer.switchCameraProjectionKey = sequence[0].key();
		}
		else {
			m_globalInfo.logger.addLog(LogLevel::Warning, m_globalInfo.localization.getString("log_binding_unauthorized", { m_globalInfo.localization.getString("editor_parameters_switch_camera_projection"), key }));
			senderWidget->setKey(m_globalInfo.editorParameters.renderer.switchCameraProjectionKey);
		}
	}
	else if (senderWidget == resetCameraKeySelect) {
		QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(key));
		if (!sequence.isEmpty() && authorizedKey(sequence[0].key())) {
			m_globalInfo.editorParameters.renderer.resetCameraKey = sequence[0].key();
			m_globalInfo.mainWindow->viewMenu->resetCameraAction->setShortcut(m_globalInfo.editorParameters.renderer.resetCameraKey);
		}
		else {
			m_globalInfo.logger.addLog(LogLevel::Warning, m_globalInfo.localization.getString("log_binding_unauthorized", { m_globalInfo.localization.getString("editor_parameters_reset_camera"), key }));
			senderWidget->setKey(m_globalInfo.editorParameters.renderer.resetCameraKey);
		}
	}
	else if (senderWidget == orthographicCameraToXMKeySelect) {
		QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(key));
		if (!sequence.isEmpty() && authorizedKey(sequence[0].key())) {
			m_globalInfo.editorParameters.renderer.orthographicCameraToXMKey = sequence[0].key();
			m_globalInfo.mainWindow->viewMenu->orthographicCameraToXMAction->setShortcut(m_globalInfo.editorParameters.renderer.orthographicCameraToXMKey);
		}
		else {
			m_globalInfo.logger.addLog(LogLevel::Warning, m_globalInfo.localization.getString("log_binding_unauthorized", { m_globalInfo.localization.getString("editor_parameters_orthographic_camera_xm"), key }));
			senderWidget->setKey(m_globalInfo.editorParameters.renderer.orthographicCameraToXMKey);
		}
	}
	else if (senderWidget == orthographicCameraToXPKeySelect) {
		QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(key));
		if (!sequence.isEmpty() && authorizedKey(sequence[0].key())) {
			m_globalInfo.editorParameters.renderer.orthographicCameraToXPKey = sequence[0].key();
			m_globalInfo.mainWindow->viewMenu->orthographicCameraToXPAction->setShortcut(m_globalInfo.editorParameters.renderer.orthographicCameraToXPKey);
		}
		else {
			m_globalInfo.logger.addLog(LogLevel::Warning, m_globalInfo.localization.getString("log_binding_unauthorized", { m_globalInfo.localization.getString("editor_parameters_orthographic_camera_xp"), key }));
			senderWidget->setKey(m_globalInfo.editorParameters.renderer.orthographicCameraToXPKey);
		}
	}
	else if (senderWidget == orthographicCameraToYMKeySelect) {
		QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(key));
		if (!sequence.isEmpty() && authorizedKey(sequence[0].key())) {
			m_globalInfo.editorParameters.renderer.orthographicCameraToYMKey = sequence[0].key();
			m_globalInfo.mainWindow->viewMenu->orthographicCameraToYMAction->setShortcut(m_globalInfo.editorParameters.renderer.orthographicCameraToYMKey);
		}
		else {
			m_globalInfo.logger.addLog(LogLevel::Warning, m_globalInfo.localization.getString("log_binding_unauthorized", { m_globalInfo.localization.getString("editor_parameters_orthographic_camera_ym"), key }));
			senderWidget->setKey(m_globalInfo.editorParameters.renderer.orthographicCameraToYMKey);
		}
	}
	else if (senderWidget == orthographicCameraToYPKeySelect) {
		QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(key));
		if (!sequence.isEmpty() && authorizedKey(sequence[0].key())) {
			m_globalInfo.editorParameters.renderer.orthographicCameraToYPKey = sequence[0].key();
			m_globalInfo.mainWindow->viewMenu->orthographicCameraToYPAction->setShortcut(m_globalInfo.editorParameters.renderer.orthographicCameraToYPKey);
		}
		else {
			m_globalInfo.logger.addLog(LogLevel::Warning, m_globalInfo.localization.getString("log_binding_unauthorized", { m_globalInfo.localization.getString("editor_parameters_orthographic_camera_yp"), key }));
			senderWidget->setKey(m_globalInfo.editorParameters.renderer.orthographicCameraToYPKey);
		}
	}
	else if (senderWidget == orthographicCameraToZMKeySelect) {
		QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(key));
		if (!sequence.isEmpty() && authorizedKey(sequence[0].key())) {
			m_globalInfo.editorParameters.renderer.orthographicCameraToZMKey = sequence[0].key();
			m_globalInfo.mainWindow->viewMenu->orthographicCameraToZMAction->setShortcut(m_globalInfo.editorParameters.renderer.orthographicCameraToZMKey);
		}
		else {
			m_globalInfo.logger.addLog(LogLevel::Warning, m_globalInfo.localization.getString("log_binding_unauthorized", { m_globalInfo.localization.getString("editor_parameters_orthographic_camera_zm"), key }));
			senderWidget->setKey(m_globalInfo.editorParameters.renderer.orthographicCameraToZMKey);
		}
	}
	else if (senderWidget == orthographicCameraToZPKeySelect) {
		QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(key));
		if (!sequence.isEmpty() && authorizedKey(sequence[0].key())) {
			m_globalInfo.editorParameters.renderer.orthographicCameraToZPKey = sequence[0].key();
			m_globalInfo.mainWindow->viewMenu->orthographicCameraToZPAction->setShortcut(m_globalInfo.editorParameters.renderer.orthographicCameraToZPKey);
		}
		else {
			m_globalInfo.logger.addLog(LogLevel::Warning, m_globalInfo.localization.getString("log_binding_unauthorized", { m_globalInfo.localization.getString("editor_parameters_orthographic_camera_zp"), key }));
			senderWidget->setKey(m_globalInfo.editorParameters.renderer.orthographicCameraToZPKey);
		}
	}
	else if (senderWidget == cameraGoToEntityKeySelect) {
		QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(key));
		if (!sequence.isEmpty() && authorizedKey(sequence[0].key())) {
			m_globalInfo.editorParameters.renderer.cameraGoToEntityKey = sequence[0].key();
			m_globalInfo.mainWindow->viewMenu->cameraGoToEntityAction->setShortcut(m_globalInfo.editorParameters.renderer.cameraGoToEntityKey);
		}
		else {
			m_globalInfo.logger.addLog(LogLevel::Warning, m_globalInfo.localization.getString("log_binding_unauthorized", { m_globalInfo.localization.getString("editor_parameters_camera_go_to_entity"), key }));
			senderWidget->setKey(m_globalInfo.editorParameters.renderer.cameraGoToEntityKey);
		}
	}
	else if (senderWidget == translateEntityKeySelect) {
		QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(key));
		if (!sequence.isEmpty() && authorizedKey(sequence[0].key())) {
			m_globalInfo.editorParameters.renderer.translateEntityKey = sequence[0].key();
		}
		else {
			m_globalInfo.logger.addLog(LogLevel::Warning, m_globalInfo.localization.getString("log_binding_unauthorized", { m_globalInfo.localization.getString("editor_parameters_translate_entity"), key }));
			senderWidget->setKey(m_globalInfo.editorParameters.renderer.translateEntityKey);
		}
	}
	else if (senderWidget == rotateEntityKeySelect) {
		QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(key));
		if (!sequence.isEmpty() && authorizedKey(sequence[0].key())) {
			m_globalInfo.editorParameters.renderer.rotateEntityKey = sequence[0].key();
		}
		else {
			m_globalInfo.logger.addLog(LogLevel::Warning, m_globalInfo.localization.getString("log_binding_unauthorized", { m_globalInfo.localization.getString("editor_parameters_rotate_entity"), key }));
			senderWidget->setKey(m_globalInfo.editorParameters.renderer.rotateEntityKey);
		}
	}
	else if (senderWidget == scaleEntityKeySelect) {
		QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(key));
		if (!sequence.isEmpty() && authorizedKey(sequence[0].key())) {
			m_globalInfo.editorParameters.renderer.scaleEntityKey = sequence[0].key();
		}
		else {
			m_globalInfo.logger.addLog(LogLevel::Warning, m_globalInfo.localization.getString("log_binding_unauthorized", { m_globalInfo.localization.getString("editor_parameters_scale_entity"), key }));
			senderWidget->setKey(m_globalInfo.editorParameters.renderer.scaleEntityKey);
		}
	}
	else if (senderWidget == toggleCurrentEntityVisibilityKeySelect) {
		QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(key));
		if (!sequence.isEmpty() && authorizedKey(sequence[0].key())) {
			m_globalInfo.editorParameters.renderer.toggleCurrentEntityVisibilityKey = sequence[0].key();
			m_globalInfo.mainWindow->viewMenu->toggleCurrentEntityVisibilityAction->setShortcut(m_globalInfo.editorParameters.renderer.toggleCurrentEntityVisibilityKey);
		}
		else {
			m_globalInfo.logger.addLog(LogLevel::Warning, m_globalInfo.localization.getString("log_binding_unauthorized", { m_globalInfo.localization.getString("editor_parameters_toggle_current_entity_visibility"), key }));
			senderWidget->setKey(m_globalInfo.editorParameters.renderer.toggleCurrentEntityVisibilityKey);
		}
	}
	else if (senderWidget == toggleGridVisibilityKeySelect) {
		QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(key));
		if (!sequence.isEmpty() && authorizedKey(sequence[0].key())) {
			m_globalInfo.editorParameters.renderer.toggleGridVisibilityKey = sequence[0].key();
			m_globalInfo.mainWindow->viewMenu->toggleGridVisibilityAction->setShortcut(m_globalInfo.editorParameters.renderer.toggleGridVisibilityKey);
		}
		else {
			m_globalInfo.logger.addLog(LogLevel::Warning, m_globalInfo.localization.getString("log_binding_unauthorized", { m_globalInfo.localization.getString("editor_parameters_toggle_grid_visibility"), key }));
			senderWidget->setKey(m_globalInfo.editorParameters.renderer.toggleGridVisibilityKey);
		}
	}
	else if (senderWidget == toggleBackfaceCullingKeySelect) {
		QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(key));
		if (!sequence.isEmpty() && authorizedKey(sequence[0].key())) {
			m_globalInfo.editorParameters.renderer.toggleBackfaceCullingKey = sequence[0].key();
			m_globalInfo.mainWindow->viewMenu->toggleBackfaceCullingAction->setShortcut(m_globalInfo.editorParameters.renderer.toggleBackfaceCullingKey);
		}
		else {
			m_globalInfo.logger.addLog(LogLevel::Warning, m_globalInfo.localization.getString("log_binding_unauthorized", { m_globalInfo.localization.getString("editor_parameters_toggle_backface_culling"), key }));
			senderWidget->setKey(m_globalInfo.editorParameters.renderer.toggleBackfaceCullingKey);
		}
	}
	else if (senderWidget == toggleCamerasVisibilityKeySelect) {
		QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(key));
		if (!sequence.isEmpty() && authorizedKey(sequence[0].key())) {
			m_globalInfo.editorParameters.renderer.toggleCamerasVisibilityKey = sequence[0].key();
			m_globalInfo.mainWindow->viewMenu->toggleCamerasVisibilityAction->setShortcut(m_globalInfo.editorParameters.renderer.toggleCamerasVisibilityKey);
		}
		else {
			m_globalInfo.logger.addLog(LogLevel::Warning, m_globalInfo.localization.getString("log_binding_unauthorized", { m_globalInfo.localization.getString("editor_parameters_toggle_cameras_visibility"), key }));
			senderWidget->setKey(m_globalInfo.editorParameters.renderer.toggleCamerasVisibilityKey);
		}
	}
	else if (senderWidget == toggleLightingKeySelect) {
		QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(key));
		if (!sequence.isEmpty() && authorizedKey(sequence[0].key())) {
			m_globalInfo.editorParameters.renderer.toggleLightingKey = sequence[0].key();
			m_globalInfo.mainWindow->viewMenu->toggleLightingAction->setShortcut(m_globalInfo.editorParameters.renderer.toggleLightingKey);
		}
		else {
			m_globalInfo.logger.addLog(LogLevel::Warning, m_globalInfo.localization.getString("log_binding_unauthorized", { m_globalInfo.localization.getString("editor_parameters_toggle_lighting"), key }));
			senderWidget->setKey(m_globalInfo.editorParameters.renderer.toggleLightingKey);
		}
	}
	else if (senderWidget == toggleCollidersVisibilityKeySelect) {
		QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(key));
		if (!sequence.isEmpty() && authorizedKey(sequence[0].key())) {
			m_globalInfo.editorParameters.renderer.toggleCollidersVisibilityKey = sequence[0].key();
			m_globalInfo.mainWindow->viewMenu->toggleCollidersVisibilityAction->setShortcut(m_globalInfo.editorParameters.renderer.toggleCollidersVisibilityKey);
		}
		else {
			m_globalInfo.logger.addLog(LogLevel::Warning, m_globalInfo.localization.getString("log_binding_unauthorized", { m_globalInfo.localization.getString("editor_parameters_toggle_colliders_visibility"), key }));
			senderWidget->setKey(m_globalInfo.editorParameters.renderer.toggleCollidersVisibilityKey);
		}
	}

	save();
}

void EditorParametersWidget::onBooleanChanged(bool value) {
	QObject* senderWidget = sender();
	if (senderWidget == maintainGizmoSizeWidget) {
		m_globalInfo.editorParameters.renderer.maintainGizmoSize = value;
	}

	save();
}

void EditorParametersWidget::onScalarChanged(float value) {
	QObject* senderWidget = sender();
	if (senderWidget == cameraNearPlaneWidget) {
		m_globalInfo.editorParameters.renderer.cameraNearPlane = value;
	}
	else if (senderWidget == cameraFarPlaneWidget) {
		m_globalInfo.editorParameters.renderer.cameraFarPlane = value;
	}
	else if (senderWidget == perspectiveCameraSpeedWidget) {
		m_globalInfo.editorParameters.renderer.perspectiveCameraSpeed = value;
	}
	else if (senderWidget == orthographicCameraSpeedWidget) {
		m_globalInfo.editorParameters.renderer.orthographicCameraSpeed = value;
	}
	else if (senderWidget == cameraSensitivityWidget) {
		m_globalInfo.editorParameters.renderer.cameraSensitivity = value;
	}
	else if (senderWidget == gridCellSizeWidget) {
		m_globalInfo.editorParameters.renderer.gridCellSize = value;
	}
	else if (senderWidget == gridSubcellSizeWidget) {
		m_globalInfo.editorParameters.renderer.gridSubcellSize = value;
	}
	else if (senderWidget == gizmoSizeWidget) {
		m_globalInfo.editorParameters.renderer.gizmoSize = value;
	}

	save();
}

void EditorParametersWidget::onStringChanged(const std::string& text) {
	QObject* senderWidget = sender();
	if (senderWidget == cMakePathWidget) {
		m_globalInfo.editorParameters.build.cMakePath = text;
	}
	else if (senderWidget == codeEditorCommandWidget) {
		m_globalInfo.editorParameters.code.codeEditorCommand = text;
	}

	save();
}

void EditorParametersWidget::onVector3Changed(const nml::vec3& value) {
	Vector3Widget* senderWidget = static_cast<Vector3Widget*>(sender());

	nml::vec3 clampedValue = value;
	if (value.x < 0.0f) {
		clampedValue.x = 0.0f;
	}
	if (value.y < 0.0f) {
		clampedValue.y = 0.0f;
	}
	if (value.z < 0.0f) {
		clampedValue.z = 0.0f;
	}
	if (clampedValue != value) {
		senderWidget->setValue(clampedValue);
	}

	if (senderWidget == gizmoTranslationStepWidget) {
		m_globalInfo.editorParameters.renderer.gizmoTranslationStep = clampedValue;
	}
	else if (senderWidget == gizmoRotationStepWidget) {
		m_globalInfo.editorParameters.renderer.gizmoRotationStep = clampedValue;
	}
	else if (senderWidget == gizmoScaleStepWidget) {
		m_globalInfo.editorParameters.renderer.gizmoScaleStep = clampedValue;
	}

	save();
}

void EditorParametersWidget::onColorChanged(const nml::vec3& color) {
	QObject* senderWidget = sender();
	if (senderWidget == currentEntityOutlineColorWidget) {
		m_globalInfo.editorParameters.renderer.currentEntityOutlineColor = color;
	}
	else if (senderWidget == otherEntitiesOutlineColorWidget) {
		m_globalInfo.editorParameters.renderer.otherEntitiesOutlineColor = color;
	}

	save();
}

void EditorParametersWidget::onTemporaryColorChanged(const nml::vec3& color) {
	QObject* senderWidget = sender();
	if (senderWidget == currentEntityOutlineColorWidget) {
		m_globalInfo.editorParameters.renderer.currentEntityOutlineColor = color;
	}
	else if (senderWidget == otherEntitiesOutlineColorWidget) {
		m_globalInfo.editorParameters.renderer.otherEntitiesOutlineColor = color;
	}
}

void EditorParametersWidget::closeEvent(QCloseEvent* event) {
	emit closeWindow();
	QTabWidget::closeEvent(event);
}
