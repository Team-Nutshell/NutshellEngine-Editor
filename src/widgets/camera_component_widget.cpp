#include "camera_component_widget.h"
#include "component_title_widget.h"
#include "separator_line.h"
#include "../undo_commands/change_entity_component_command.h"
#include <QVBoxLayout>

CameraComponentWidget::CameraComponentWidget(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setLayout(new QVBoxLayout());
	layout()->setAlignment(Qt::AlignmentFlag::AlignTop);
	layout()->setContentsMargins(0, 0, 0, 0);
	layout()->addWidget(new ComponentTitleWidget(m_globalInfo, "Camera"));
	forwardWidget = new Vector3Widget(m_globalInfo, "Forward");
	layout()->addWidget(forwardWidget);
	upWidget = new Vector3Widget(m_globalInfo, "Up");
	layout()->addWidget(upWidget);
	fovWidget = new ScalarWidget(m_globalInfo, "FOV");
	layout()->addWidget(fovWidget);
	nearPlaneWidget = new ScalarWidget(m_globalInfo, "Near Plane");
	layout()->addWidget(nearPlaneWidget);
	farPlaneWidget = new ScalarWidget(m_globalInfo, "Far Plane");
	layout()->addWidget(farPlaneWidget);
	layout()->addWidget(new SeparatorLine(m_globalInfo));

	connect(forwardWidget, &Vector3Widget::valueChanged, this, &CameraComponentWidget::onVec3Updated);
	connect(upWidget, &Vector3Widget::valueChanged, this, &CameraComponentWidget::onVec3Updated);
	connect(fovWidget, &ScalarWidget::valueChanged, this, &CameraComponentWidget::onScalarUpdated);
	connect(nearPlaneWidget, &ScalarWidget::valueChanged, this, &CameraComponentWidget::onScalarUpdated);
	connect(farPlaneWidget, &ScalarWidget::valueChanged, this, &CameraComponentWidget::onScalarUpdated);
	connect(&globalInfo.signalEmitter, &SignalEmitter::selectEntitySignal, this, &CameraComponentWidget::onSelectEntity);
	connect(&globalInfo.signalEmitter, &SignalEmitter::addEntityCameraSignal, this, &CameraComponentWidget::onAddEntityCamera);
	connect(&globalInfo.signalEmitter, &SignalEmitter::removeEntityCameraSignal, this, &CameraComponentWidget::onRemoveEntityCamera);
	connect(&globalInfo.signalEmitter, &SignalEmitter::changeEntityCameraSignal, this, &CameraComponentWidget::onChangeEntityCamera);
}

void CameraComponentWidget::updateWidgets(const Camera& camera) {
	forwardWidget->value = camera.forward;
	forwardWidget->xLineEdit->setText(QString::number(camera.forward.x, 'g', 3));
	forwardWidget->yLineEdit->setText(QString::number(camera.forward.y, 'g', 3));
	forwardWidget->zLineEdit->setText(QString::number(camera.forward.z, 'g', 3));
	upWidget->value = camera.up;
	upWidget->xLineEdit->setText(QString::number(camera.up.x, 'g', 3));
	upWidget->yLineEdit->setText(QString::number(camera.up.y, 'g', 3));
	upWidget->zLineEdit->setText(QString::number(camera.up.z, 'g', 3));
	fovWidget->value = camera.fov;
	fovWidget->valueLineEdit->setText(QString::number(camera.fov, 'g', 3));
	nearPlaneWidget->value = camera.nearPlane;
	nearPlaneWidget->valueLineEdit->setText(QString::number(camera.nearPlane, 'g', 3));
	farPlaneWidget->value = camera.farPlane;
	farPlaneWidget->valueLineEdit->setText(QString::number(camera.farPlane, 'g', 3));
}

void CameraComponentWidget::onSelectEntity() {
	if ((m_globalInfo.currentEntityID != NO_ENTITY) && m_globalInfo.entities[m_globalInfo.currentEntityID].camera.has_value()) {
		show();
		updateWidgets(m_globalInfo.entities[m_globalInfo.currentEntityID].camera.value());
	}
	else {
		hide();
	}
}

void CameraComponentWidget::onAddEntityCamera(EntityID entityID) {
	if (entityID == m_globalInfo.currentEntityID) {
		Camera camera = m_globalInfo.entities[m_globalInfo.currentEntityID].camera.value();
		updateWidgets(camera);
		show();
	}
}

void CameraComponentWidget::onRemoveEntityCamera(EntityID entityID) {
	if (entityID == m_globalInfo.currentEntityID) {
		hide();
	}
}

void CameraComponentWidget::onChangeEntityCamera(EntityID entityID, const Camera& camera) {
	if (sender() != this) {
		if (entityID == m_globalInfo.currentEntityID) {
			updateWidgets(camera);
		}
	}
}

void CameraComponentWidget::onVec3Updated(const nml::vec3& value) {
	Camera newCamera = m_globalInfo.entities[m_globalInfo.currentEntityID].camera.value();
	if (sender() == forwardWidget) {
		newCamera.forward = value;
	}
	else if (sender() == upWidget) {
		newCamera.up = value;
	}
	m_globalInfo.undoStack->push(new ChangeEntityComponentCommand(m_globalInfo, m_globalInfo.currentEntityID, "Camera", &newCamera));
}

void CameraComponentWidget::onScalarUpdated(float value) {
	Camera newCamera = m_globalInfo.entities[m_globalInfo.currentEntityID].camera.value();
	if (sender() == fovWidget) {
		newCamera.fov = value;
	}
	else if (sender() == nearPlaneWidget) {
		newCamera.nearPlane = value;
	}
	else if (sender() == farPlaneWidget) {
		newCamera.farPlane = value;
	}
	m_globalInfo.undoStack->push(new ChangeEntityComponentCommand(m_globalInfo, m_globalInfo.currentEntityID, "Camera", &newCamera));
}
