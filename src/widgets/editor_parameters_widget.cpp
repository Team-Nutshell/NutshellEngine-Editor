#include "editor_parameters_widget.h"
#include "main_window.h"
#include "separator_line.h"
#include <QGridLayout>
#include <QVBoxLayout>
#include <fstream>

EditorParametersWidget::EditorParametersWidget(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	resize(1000, 450);
	setWindowTitle("NutshellEngine - Editor Parameters");
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
	cameraForwardKeySelect = new KeySelectWidget(m_globalInfo, "Camera Forward", cameraForwardKeySequence.toString().toStdString());
	cameraForwardKeySelect->layout()->setAlignment(cameraForwardKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	rendererKeyGridLayout->addWidget(cameraForwardKeySelect, 0, 0);

	QKeySequence cameraBackwardKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.cameraBackwardKey);
	cameraBackwardKeySelect = new KeySelectWidget(m_globalInfo, "Camera Backward", cameraBackwardKeySequence.toString().toStdString());
	cameraBackwardKeySelect->layout()->setAlignment(cameraBackwardKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	rendererKeyGridLayout->addWidget(cameraBackwardKeySelect, 1, 0);

	QKeySequence cameraLeftKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.cameraLeftKey);
	cameraLeftKeySelect = new KeySelectWidget(m_globalInfo, "Camera Left", cameraLeftKeySequence.toString().toStdString());
	cameraLeftKeySelect->layout()->setAlignment(cameraLeftKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	rendererKeyGridLayout->addWidget(cameraLeftKeySelect, 2, 0);

	QKeySequence cameraRightKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.cameraRightKey);
	cameraRightKeySelect = new KeySelectWidget(m_globalInfo, "Camera Right", cameraRightKeySequence.toString().toStdString());
	cameraRightKeySelect->layout()->setAlignment(cameraRightKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	rendererKeyGridLayout->addWidget(cameraRightKeySelect, 3, 0);

	QKeySequence cameraUpKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.cameraUpKey);
	cameraUpKeySelect = new KeySelectWidget(m_globalInfo, "Camera Up", cameraUpKeySequence.toString().toStdString());
	cameraUpKeySelect->layout()->setAlignment(cameraUpKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	rendererKeyGridLayout->addWidget(cameraUpKeySelect, 4, 0);

	QKeySequence cameraDownKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.cameraDownKey);
	cameraDownKeySelect = new KeySelectWidget(m_globalInfo, "Camera Down", cameraDownKeySequence.toString().toStdString());
	cameraDownKeySelect->layout()->setAlignment(cameraDownKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	rendererKeyGridLayout->addWidget(cameraDownKeySelect, 5, 0);

	QKeySequence switchCameraProjectionKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.switchCameraProjectionKey);
	switchCameraProjectionKeySelect = new KeySelectWidget(m_globalInfo, "Switch Camera Projection", switchCameraProjectionKeySequence.toString().toStdString());
	switchCameraProjectionKeySelect->layout()->setAlignment(switchCameraProjectionKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	rendererKeyGridLayout->addWidget(switchCameraProjectionKeySelect, 6, 0);

	QKeySequence resetCameraKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.resetCameraKey);
	resetCameraKeySelect = new KeySelectWidget(m_globalInfo, "Switch Camera Projection", resetCameraKeySequence.toString().toStdString());
	resetCameraKeySelect->layout()->setAlignment(resetCameraKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	rendererKeyGridLayout->addWidget(resetCameraKeySelect, 7, 0);

	QKeySequence orthographicCameraToXMKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.orthographicCameraToXMKey);
	orthographicCameraToXMKeySelect = new KeySelectWidget(m_globalInfo, "Orthographic Camera To X-", orthographicCameraToXMKeySequence.toString().toStdString());
	orthographicCameraToXMKeySelect->layout()->setAlignment(orthographicCameraToXMKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	rendererKeyGridLayout->addWidget(orthographicCameraToXMKeySelect, 0, 1);

	QKeySequence orthographicCameraToXPKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.orthographicCameraToXPKey);
	orthographicCameraToXPKeySelect = new KeySelectWidget(m_globalInfo, "Orthographic Camera To X+", orthographicCameraToXPKeySequence.toString().toStdString());
	orthographicCameraToXPKeySelect->layout()->setAlignment(orthographicCameraToXPKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	rendererKeyGridLayout->addWidget(orthographicCameraToXPKeySelect, 1, 1);

	QKeySequence orthographicCameraToYMKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.orthographicCameraToYMKey);
	orthographicCameraToYMKeySelect = new KeySelectWidget(m_globalInfo, "Orthographic Camera To Y-", orthographicCameraToYMKeySequence.toString().toStdString());
	orthographicCameraToYMKeySelect->layout()->setAlignment(orthographicCameraToYMKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	rendererKeyGridLayout->addWidget(orthographicCameraToYMKeySelect, 2, 1);

	QKeySequence orthographicCameraToYPKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.orthographicCameraToYPKey);
	orthographicCameraToYPKeySelect = new KeySelectWidget(m_globalInfo, "Orthographic Camera To Y+", orthographicCameraToYPKeySequence.toString().toStdString());
	orthographicCameraToYPKeySelect->layout()->setAlignment(orthographicCameraToYPKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	rendererKeyGridLayout->addWidget(orthographicCameraToYPKeySelect, 3, 1);

	QKeySequence orthographicCameraToZMKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.orthographicCameraToZMKey);
	orthographicCameraToZMKeySelect = new KeySelectWidget(m_globalInfo, "Orthographic Camera To Z-", orthographicCameraToZMKeySequence.toString().toStdString());
	orthographicCameraToZMKeySelect->layout()->setAlignment(orthographicCameraToZMKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	rendererKeyGridLayout->addWidget(orthographicCameraToZMKeySelect, 4, 1);

	QKeySequence orthographicCameraToZPKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.orthographicCameraToZPKey);
	orthographicCameraToZPKeySelect = new KeySelectWidget(m_globalInfo, "Orthographic Camera To Z+", orthographicCameraToZPKeySequence.toString().toStdString());
	orthographicCameraToZPKeySelect->layout()->setAlignment(orthographicCameraToZPKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	rendererKeyGridLayout->addWidget(orthographicCameraToZPKeySelect, 5, 1);

	QKeySequence translateEntityKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.translateEntityKey);
	translateEntityKeySelect = new KeySelectWidget(m_globalInfo, "Translate Entity", translateEntityKeySequence.toString().toStdString());
	translateEntityKeySelect->layout()->setAlignment(translateEntityKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	rendererKeyGridLayout->addWidget(translateEntityKeySelect, 6, 1);

	QKeySequence rotateEntityKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.rotateEntityKey);
	rotateEntityKeySelect = new KeySelectWidget(m_globalInfo, "Rotate Entity", rotateEntityKeySequence.toString().toStdString());
	rotateEntityKeySelect->layout()->setAlignment(rotateEntityKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	rendererKeyGridLayout->addWidget(rotateEntityKeySelect, 7, 1);

	QKeySequence scaleEntityKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.scaleEntityKey);
	scaleEntityKeySelect = new KeySelectWidget(m_globalInfo, "Scale Entity", scaleEntityKeySequence.toString().toStdString());
	scaleEntityKeySelect->layout()->setAlignment(scaleEntityKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	rendererKeyGridLayout->addWidget(scaleEntityKeySelect, 0, 2);

	QKeySequence toggleCurrentEntityVisibilityKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.toggleCurrentEntityVisibilityKey);
	toggleCurrentEntityVisibilityKeySelect = new KeySelectWidget(m_globalInfo, "Toggle Current Entity Visibility", toggleCurrentEntityVisibilityKeySequence.toString().toStdString());
	toggleCurrentEntityVisibilityKeySelect->layout()->setAlignment(toggleCurrentEntityVisibilityKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	rendererKeyGridLayout->addWidget(toggleCurrentEntityVisibilityKeySelect, 1, 2);

	QKeySequence toggleGridVisibilityKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.toggleGridVisibilityKey);
	toggleGridVisibilityKeySelect = new KeySelectWidget(m_globalInfo, "Toggle Grid Visibility", toggleGridVisibilityKeySequence.toString().toStdString());
	toggleGridVisibilityKeySelect->layout()->setAlignment(toggleGridVisibilityKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	rendererKeyGridLayout->addWidget(toggleGridVisibilityKeySelect, 2, 2);

	QKeySequence toggleBackfaceCullingKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.toggleBackfaceCullingKey);
	toggleBackfaceCullingKeySelect = new KeySelectWidget(m_globalInfo, "Toggle Backface Culling", toggleBackfaceCullingKeySequence.toString().toStdString());
	toggleBackfaceCullingKeySelect->layout()->setAlignment(toggleBackfaceCullingKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	rendererKeyGridLayout->addWidget(toggleBackfaceCullingKeySelect, 3, 2);

	QKeySequence toggleCamerasVisibilityKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.toggleCamerasVisibilityKey);
	toggleCamerasVisibilityKeySelect = new KeySelectWidget(m_globalInfo, "Toggle Cameras Visibility", toggleCamerasVisibilityKeySequence.toString().toStdString());
	toggleCamerasVisibilityKeySelect->layout()->setAlignment(toggleCamerasVisibilityKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	rendererKeyGridLayout->addWidget(toggleCamerasVisibilityKeySelect, 4, 2);

	QKeySequence toggleLightingKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.toggleLightingKey);
	toggleLightingKeySelect = new KeySelectWidget(m_globalInfo, "Toggle Lighting", toggleLightingKeySequence.toString().toStdString());
	toggleLightingKeySelect->layout()->setAlignment(toggleLightingKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	rendererKeyGridLayout->addWidget(toggleLightingKeySelect, 5, 2);

	QKeySequence toggleCollidersVisibilityKeySequence = QKeySequence(m_globalInfo.editorParameters.renderer.toggleCollidersVisibilityKey);
	toggleCollidersVisibilityKeySelect = new KeySelectWidget(m_globalInfo, "Toggle Colliders Visibility", toggleCollidersVisibilityKeySequence.toString().toStdString());
	toggleCollidersVisibilityKeySelect->layout()->setAlignment(toggleCollidersVisibilityKeySelect->button, Qt::AlignmentFlag::AlignLeft);
	rendererKeyGridLayout->addWidget(toggleCollidersVisibilityKeySelect, 6, 2);

	rendererVerticalLayout->addWidget(new SeparatorLine(m_globalInfo));

	QWidget* rendererCameraGridLayoutWidget = new QWidget();
	QGridLayout* rendererCameraGridLayout = new QGridLayout();
	rendererCameraGridLayout->setContentsMargins(0, 0, 0, 0);
	rendererCameraGridLayoutWidget->setLayout(rendererCameraGridLayout);
	rendererVerticalLayout->addWidget(rendererCameraGridLayoutWidget);

	cameraNearPlaneWidget = new ScalarWidget(m_globalInfo, "Camera Near Plane");
	cameraNearPlaneWidget->valueLineEdit->setText(QString::number(m_globalInfo.editorParameters.renderer.cameraNearPlane, 'f', 3));
	cameraNearPlaneWidget->layout()->setAlignment(cameraNearPlaneWidget->nameLabel, Qt::AlignmentFlag::AlignRight);
	cameraNearPlaneWidget->layout()->setAlignment(cameraNearPlaneWidget->valueLineEdit, Qt::AlignmentFlag::AlignLeft);
	rendererCameraGridLayout->addWidget(cameraNearPlaneWidget, 0, 0);

	cameraFarPlaneWidget = new ScalarWidget(m_globalInfo, "Camera Far Plane");
	cameraFarPlaneWidget->valueLineEdit->setText(QString::number(m_globalInfo.editorParameters.renderer.cameraFarPlane, 'f', 3));
	cameraFarPlaneWidget->layout()->setAlignment(cameraFarPlaneWidget->nameLabel, Qt::AlignmentFlag::AlignRight);
	cameraFarPlaneWidget->layout()->setAlignment(cameraFarPlaneWidget->valueLineEdit, Qt::AlignmentFlag::AlignLeft);
	rendererCameraGridLayout->addWidget(cameraFarPlaneWidget, 1, 0);

	cameraSpeedWidget = new ScalarWidget(m_globalInfo, "Camera Speed");
	cameraSpeedWidget->valueLineEdit->setText(QString::number(m_globalInfo.editorParameters.renderer.cameraSpeed, 'f', 3));
	cameraSpeedWidget->layout()->setAlignment(cameraSpeedWidget->nameLabel, Qt::AlignmentFlag::AlignRight);
	cameraSpeedWidget->layout()->setAlignment(cameraSpeedWidget->valueLineEdit, Qt::AlignmentFlag::AlignLeft);
	rendererCameraGridLayout->addWidget(cameraSpeedWidget, 0, 1);

	cameraSensitivityWidget = new ScalarWidget(m_globalInfo, "Camera Sensitivity");
	cameraSensitivityWidget->valueLineEdit->setText(QString::number(m_globalInfo.editorParameters.renderer.cameraSensitivity, 'f', 3));
	cameraSensitivityWidget->layout()->setAlignment(cameraSensitivityWidget->nameLabel, Qt::AlignmentFlag::AlignRight);
	cameraSensitivityWidget->layout()->setAlignment(cameraSensitivityWidget->valueLineEdit, Qt::AlignmentFlag::AlignLeft);
	rendererCameraGridLayout->addWidget(cameraSensitivityWidget, 1, 1);

	gridScaleWidget = new ScalarWidget(m_globalInfo, "Grid Scale");
	gridScaleWidget->valueLineEdit->setText(QString::number(m_globalInfo.editorParameters.renderer.gridScale, 'f', 3));
	gridScaleWidget->layout()->setAlignment(gridScaleWidget->nameLabel, Qt::AlignmentFlag::AlignRight);
	gridScaleWidget->layout()->setAlignment(gridScaleWidget->valueLineEdit, Qt::AlignmentFlag::AlignLeft);
	rendererCameraGridLayout->addWidget(gridScaleWidget, 0, 2);

	rendererVerticalLayout->addWidget(new SeparatorLine(m_globalInfo));

	QWidget* rendererGuizmoStepLayoutWidget = new QWidget();
	QVBoxLayout* rendererGuizmoStepLayout = new QVBoxLayout();
	rendererGuizmoStepLayout->setContentsMargins(0, 0, 0, 0);
	rendererGuizmoStepLayoutWidget->setLayout(rendererGuizmoStepLayout);
	rendererVerticalLayout->addWidget(rendererGuizmoStepLayoutWidget);

	guizmoTranslationStepWidget = new Vector3Widget(m_globalInfo, "Guizmo Translation Step");
	guizmoTranslationStepWidget->setValue(m_globalInfo.editorParameters.renderer.guizmoTranslationStep);
	guizmoTranslationStepWidget->layout()->setAlignment(guizmoTranslationStepWidget->nameLabel, Qt::AlignmentFlag::AlignLeft);
	guizmoTranslationStepWidget->layout()->setAlignment(guizmoTranslationStepWidget->xLineEdit, Qt::AlignmentFlag::AlignLeft);
	guizmoTranslationStepWidget->layout()->setAlignment(guizmoTranslationStepWidget->yLineEdit, Qt::AlignmentFlag::AlignLeft);
	guizmoTranslationStepWidget->layout()->setAlignment(guizmoTranslationStepWidget->zLineEdit, Qt::AlignmentFlag::AlignLeft);
	rendererGuizmoStepLayout->addWidget(guizmoTranslationStepWidget);

	guizmoRotationStepWidget = new Vector3Widget(m_globalInfo, "Guizmo Rotation Step");
	guizmoRotationStepWidget->setValue(m_globalInfo.editorParameters.renderer.guizmoRotationStep);
	guizmoRotationStepWidget->layout()->setAlignment(guizmoRotationStepWidget->nameLabel, Qt::AlignmentFlag::AlignLeft);
	guizmoRotationStepWidget->layout()->setAlignment(guizmoRotationStepWidget->xLineEdit, Qt::AlignmentFlag::AlignLeft);
	guizmoRotationStepWidget->layout()->setAlignment(guizmoRotationStepWidget->yLineEdit, Qt::AlignmentFlag::AlignLeft);
	guizmoRotationStepWidget->layout()->setAlignment(guizmoRotationStepWidget->zLineEdit, Qt::AlignmentFlag::AlignLeft);
	rendererGuizmoStepLayout->addWidget(guizmoRotationStepWidget);

	guizmoScaleStepWidget = new Vector3Widget(m_globalInfo, "Guizmo Scale Step");
	guizmoScaleStepWidget->setValue(m_globalInfo.editorParameters.renderer.guizmoScaleStep);
	guizmoScaleStepWidget->layout()->setAlignment(guizmoScaleStepWidget->nameLabel, Qt::AlignmentFlag::AlignLeft);
	guizmoScaleStepWidget->layout()->setAlignment(guizmoScaleStepWidget->xLineEdit, Qt::AlignmentFlag::AlignLeft);
	guizmoScaleStepWidget->layout()->setAlignment(guizmoScaleStepWidget->yLineEdit, Qt::AlignmentFlag::AlignLeft);
	guizmoScaleStepWidget->layout()->setAlignment(guizmoScaleStepWidget->zLineEdit, Qt::AlignmentFlag::AlignLeft);
	rendererGuizmoStepLayout->addWidget(guizmoScaleStepWidget);

	rendererVerticalLayout->addWidget(new SeparatorLine(m_globalInfo));

	currentEntityOutlineColorWidget = new ColorPickerWidget(m_globalInfo, "Current Entity Outline Color", nml::vec3(m_globalInfo.editorParameters.renderer.currentEntityOutlineColor));
	currentEntityOutlineColorWidget->layout()->setAlignment(currentEntityOutlineColorWidget->nameLabel, Qt::AlignmentFlag::AlignRight);
	currentEntityOutlineColorWidget->layout()->setAlignment(currentEntityOutlineColorWidget->colorButton, Qt::AlignmentFlag::AlignLeft);
	rendererVerticalLayout->addWidget(currentEntityOutlineColorWidget);

	otherEntitiesOutlineColorWidget = new ColorPickerWidget(m_globalInfo, "Other selected Entities Outline Color", nml::vec3(m_globalInfo.editorParameters.renderer.otherEntitiesOutlineColor));
	otherEntitiesOutlineColorWidget->layout()->setAlignment(otherEntitiesOutlineColorWidget->nameLabel, Qt::AlignmentFlag::AlignRight);
	otherEntitiesOutlineColorWidget->layout()->setAlignment(otherEntitiesOutlineColorWidget->colorButton, Qt::AlignmentFlag::AlignLeft);
	rendererVerticalLayout->addWidget(otherEntitiesOutlineColorWidget);

	addTab(rendererParametersTab, "Renderer");

	buildParametersTab = new QTabWidget();
	buildParametersTab->setLayout(new QVBoxLayout());
	buildParametersTab->layout()->setAlignment(Qt::AlignmentFlag::AlignTop);
	cMakePathWidget = new StringWidget(m_globalInfo, "CMake executable");
	cMakePathWidget->setText(m_globalInfo.editorParameters.build.cMakePath);
	cMakePathWidget->valueLineEdit->setFixedWidth(400);
	buildParametersTab->layout()->addWidget(cMakePathWidget);

	addTab(buildParametersTab, "Build");

	codeParametersTab = new QTabWidget();
	codeParametersTab->setLayout(new QVBoxLayout());
	codeParametersTab->layout()->setAlignment(Qt::AlignmentFlag::AlignTop);
	codeEditorCommandWidget = new StringWidget(m_globalInfo, "Code editor command (use \"${FILE_PATH}\" for file path)");
	codeEditorCommandWidget->setText(m_globalInfo.editorParameters.code.codeEditorCommand);
	codeEditorCommandWidget->valueLineEdit->setFixedWidth(400);
	codeParametersTab->layout()->addWidget(codeEditorCommandWidget);

	addTab(codeParametersTab, "Code");

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
	connect(cameraSpeedWidget, &ScalarWidget::valueChanged, this, &EditorParametersWidget::onScalarChanged);
	connect(cameraSensitivityWidget, &ScalarWidget::valueChanged, this, &EditorParametersWidget::onScalarChanged);
	connect(gridScaleWidget, &ScalarWidget::valueChanged, this, &EditorParametersWidget::onScalarChanged);
	connect(guizmoTranslationStepWidget, &Vector3Widget::valueChanged, this, &EditorParametersWidget::onVector3Changed);
	connect(guizmoRotationStepWidget, &Vector3Widget::valueChanged, this, &EditorParametersWidget::onVector3Changed);
	connect(guizmoScaleStepWidget, &Vector3Widget::valueChanged, this, &EditorParametersWidget::onVector3Changed);
	connect(currentEntityOutlineColorWidget, &ColorPickerWidget::colorChanged, this, &EditorParametersWidget::onColorChanged);
	connect(otherEntitiesOutlineColorWidget, &ColorPickerWidget::colorChanged, this, &EditorParametersWidget::onColorChanged);
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
			m_globalInfo.logger.addLog(LogLevel::Warning, "Camera Forward cannot be bound to the " + key + " key (Unauthorized key).");
			senderWidget->setKey(m_globalInfo.editorParameters.renderer.cameraForwardKey);
		}
	}
	else if (senderWidget == cameraBackwardKeySelect) {
		QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(key));
		if (!sequence.isEmpty() && authorizedKey(sequence[0].key())) {
			m_globalInfo.editorParameters.renderer.cameraBackwardKey = sequence[0].key();
		}
		else {
			m_globalInfo.logger.addLog(LogLevel::Warning, "Camera Backward cannot be bound to the " + key + " key (Unauthorized key).");
			senderWidget->setKey(m_globalInfo.editorParameters.renderer.cameraBackwardKey);
		}
	}
	else if (senderWidget == cameraLeftKeySelect) {
		QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(key));
		if (!sequence.isEmpty() && authorizedKey(sequence[0].key())) {
			m_globalInfo.editorParameters.renderer.cameraLeftKey = sequence[0].key();
		}
		else {
			m_globalInfo.logger.addLog(LogLevel::Warning, "Camera Left cannot be bound to the " + key + " key (Unauthorized key).");
			senderWidget->setKey(m_globalInfo.editorParameters.renderer.cameraLeftKey);
		}
	}
	else if (senderWidget == cameraRightKeySelect) {
		QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(key));
		if (!sequence.isEmpty() && authorizedKey(sequence[0].key())) {
			m_globalInfo.editorParameters.renderer.cameraRightKey = sequence[0].key();
		}
		else {
			m_globalInfo.logger.addLog(LogLevel::Warning, "Camera Right cannot be bound to the " + key + " key (Unauthorized key).");
			senderWidget->setKey(m_globalInfo.editorParameters.renderer.cameraRightKey);
		}
	}
	else if (senderWidget == cameraUpKeySelect) {
		QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(key));
		if (!sequence.isEmpty() && authorizedKey(sequence[0].key())) {
			m_globalInfo.editorParameters.renderer.cameraUpKey = sequence[0].key();
		}
		else {
			m_globalInfo.logger.addLog(LogLevel::Warning, "Camera Up cannot be bound to the " + key + " key (Unauthorized key).");
			senderWidget->setKey(m_globalInfo.editorParameters.renderer.cameraUpKey);
		}
	}
	else if (senderWidget == cameraDownKeySelect) {
		QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(key));
		if (!sequence.isEmpty() && authorizedKey(sequence[0].key())) {
			m_globalInfo.editorParameters.renderer.cameraDownKey = sequence[0].key();
		}
		else {
			m_globalInfo.logger.addLog(LogLevel::Warning, "Camera Down cannot be bound to the " + key + " key (Unauthorized key).");
			senderWidget->setKey(m_globalInfo.editorParameters.renderer.cameraDownKey);
		}
	}
	else if (senderWidget == switchCameraProjectionKeySelect) {
		QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(key));
		if (!sequence.isEmpty() && authorizedKey(sequence[0].key())) {
			m_globalInfo.editorParameters.renderer.switchCameraProjectionKey = sequence[0].key();
		}
		else {
			m_globalInfo.logger.addLog(LogLevel::Warning, "Switch Camera Projection cannot be bound to the " + key + " key (Unauthorized key).");
			senderWidget->setKey(m_globalInfo.editorParameters.renderer.switchCameraProjectionKey);
		}
	}
	else if (senderWidget == resetCameraKeySelect) {
		QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(key));
		if (!sequence.isEmpty() && authorizedKey(sequence[0].key())) {
			m_globalInfo.editorParameters.renderer.resetCameraKey = sequence[0].key();
		}
		else {
			m_globalInfo.logger.addLog(LogLevel::Warning, "Reset Camera cannot be bound to the " + key + " key (Unauthorized key).");
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
			m_globalInfo.logger.addLog(LogLevel::Warning, "Orthographic Camera To X- cannot be bound to the " + key + " key (Unauthorized key).");
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
			m_globalInfo.logger.addLog(LogLevel::Warning, "Orthographic Camera To X+ cannot be bound to the " + key + " key (Unauthorized key).");
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
			m_globalInfo.logger.addLog(LogLevel::Warning, "Orthographic Camera To Y- cannot be bound to the " + key + " key (Unauthorized key).");
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
			m_globalInfo.logger.addLog(LogLevel::Warning, "Orthographic Camera To Y+ cannot be bound to the " + key + " key (Unauthorized key).");
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
			m_globalInfo.logger.addLog(LogLevel::Warning, "Orthographic Camera To Z- cannot be bound to the " + key + " key (Unauthorized key).");
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
			m_globalInfo.logger.addLog(LogLevel::Warning, "Orthographic Camera To Z+ cannot be bound to the " + key + " key (Unauthorized key).");
			senderWidget->setKey(m_globalInfo.editorParameters.renderer.orthographicCameraToZPKey);
		}
	}
	else if (senderWidget == translateEntityKeySelect) {
		QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(key));
		if (!sequence.isEmpty() && authorizedKey(sequence[0].key())) {
			m_globalInfo.editorParameters.renderer.translateEntityKey = sequence[0].key();
		}
		else {
			m_globalInfo.logger.addLog(LogLevel::Warning, "Translate Entity cannot be bound to the " + key + " key (Unauthorized key).");
			senderWidget->setKey(m_globalInfo.editorParameters.renderer.translateEntityKey);
		}
	}
	else if (senderWidget == rotateEntityKeySelect) {
		QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(key));
		if (!sequence.isEmpty() && authorizedKey(sequence[0].key())) {
			m_globalInfo.editorParameters.renderer.rotateEntityKey = sequence[0].key();
		}
		else {
			m_globalInfo.logger.addLog(LogLevel::Warning, "Rotate Entity cannot be bound to the " + key + " key (Unauthorized key).");
			senderWidget->setKey(m_globalInfo.editorParameters.renderer.rotateEntityKey);
		}
	}
	else if (senderWidget == scaleEntityKeySelect) {
		QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(key));
		if (!sequence.isEmpty() && authorizedKey(sequence[0].key())) {
			m_globalInfo.editorParameters.renderer.scaleEntityKey = sequence[0].key();
		}
		else {
			m_globalInfo.logger.addLog(LogLevel::Warning, "Scale Entity cannot be bound to the " + key + " key (Unauthorized key).");
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
			m_globalInfo.logger.addLog(LogLevel::Warning, "Toggle Current Entity Visibility cannot be bound to the " + key + " key (Unauthorized key).");
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
			m_globalInfo.logger.addLog(LogLevel::Warning, "Toggle Grid Visibility cannot be bound to the " + key + " key (Unauthorized key).");
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
			m_globalInfo.logger.addLog(LogLevel::Warning, "Toggle Backface Culling cannot be bound to the " + key + " key (Unauthorized key).");
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
			m_globalInfo.logger.addLog(LogLevel::Warning, "Toggle Cameras Visibility cannot be bound to the " + key + " key (Unauthorized key).");
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
			m_globalInfo.logger.addLog(LogLevel::Warning, "Toggle Lighting cannot be bound to the " + key + " key (Unauthorized key).");
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
			m_globalInfo.logger.addLog(LogLevel::Warning, "Toggle Colliders Visibility cannot be bound to the " + key + " key (Unauthorized key).");
			senderWidget->setKey(m_globalInfo.editorParameters.renderer.toggleCollidersVisibilityKey);
		}
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
	else if (senderWidget == cameraSpeedWidget) {
		m_globalInfo.editorParameters.renderer.cameraSpeed = value;
	}
	else if (senderWidget == cameraSensitivityWidget) {
		m_globalInfo.editorParameters.renderer.cameraSensitivity = value;
	}
	else if (senderWidget == gridScaleWidget) {
		m_globalInfo.editorParameters.renderer.gridScale = value;
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

	if (senderWidget == guizmoTranslationStepWidget) {
		m_globalInfo.editorParameters.renderer.guizmoTranslationStep = clampedValue;
	}
	else if (senderWidget == guizmoRotationStepWidget) {
		m_globalInfo.editorParameters.renderer.guizmoRotationStep = clampedValue;
	}
	else if (senderWidget == guizmoScaleStepWidget) {
		m_globalInfo.editorParameters.renderer.guizmoScaleStep = clampedValue;
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