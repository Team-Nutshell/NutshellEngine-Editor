#include "camera_component_widget.h"
#include "component_title_widget.h"
#include "separator_line.h"
#include "../common/save_title_changer.h"
#include "../undo_commands/change_entity_component_command.h"
#include "../widgets/main_window.h"
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

	connect(forwardWidget, &Vector3Widget::valueChanged, this, &CameraComponentWidget::onVec3Changed);
	connect(upWidget, &Vector3Widget::valueChanged, this, &CameraComponentWidget::onVec3Changed);
	connect(fovWidget, &ScalarWidget::valueChanged, this, &CameraComponentWidget::onScalarChanged);
	connect(nearPlaneWidget, &ScalarWidget::valueChanged, this, &CameraComponentWidget::onScalarChanged);
	connect(farPlaneWidget, &ScalarWidget::valueChanged, this, &CameraComponentWidget::onScalarChanged);
	connect(&globalInfo.signalEmitter, &SignalEmitter::selectEntitySignal, this, &CameraComponentWidget::onEntitySelected);
	connect(&globalInfo.signalEmitter, &SignalEmitter::addEntityCameraSignal, this, &CameraComponentWidget::onAddEntityCamera);
	connect(&globalInfo.signalEmitter, &SignalEmitter::removeEntityCameraSignal, this, &CameraComponentWidget::onRemoveEntityCamera);
	connect(&globalInfo.signalEmitter, &SignalEmitter::changeEntityCameraSignal, this, &CameraComponentWidget::onChangeEntityCamera);
}

void CameraComponentWidget::updateWidgets(const Camera& camera) {
	forwardWidget->value = camera.forward;
	forwardWidget->xLineEdit->setText(QString::number(camera.forward.x, 'f', 3));
	forwardWidget->yLineEdit->setText(QString::number(camera.forward.y, 'f', 3));
	forwardWidget->zLineEdit->setText(QString::number(camera.forward.z, 'f', 3));
	upWidget->value = camera.up;
	upWidget->xLineEdit->setText(QString::number(camera.up.x, 'f', 3));
	upWidget->yLineEdit->setText(QString::number(camera.up.y, 'f', 3));
	upWidget->zLineEdit->setText(QString::number(camera.up.z, 'f', 3));
	fovWidget->value = camera.fov;
	fovWidget->valueLineEdit->setText(QString::number(camera.fov, 'f', 3));
	nearPlaneWidget->value = camera.nearPlane;
	nearPlaneWidget->valueLineEdit->setText(QString::number(camera.nearPlane, 'f', 3));
	farPlaneWidget->value = camera.farPlane;
	farPlaneWidget->valueLineEdit->setText(QString::number(camera.farPlane, 'f', 3));
}

void CameraComponentWidget::onEntitySelected() {
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
		const Camera& camera = m_globalInfo.entities[m_globalInfo.currentEntityID].camera.value();
		updateWidgets(camera);
		show();
	}

	SaveTitleChanger::change(reinterpret_cast<MainWindow*>(m_globalInfo.mainWindow));
}

void CameraComponentWidget::onRemoveEntityCamera(EntityID entityID) {
	if (entityID == m_globalInfo.currentEntityID) {
		hide();
	}

	SaveTitleChanger::change(reinterpret_cast<MainWindow*>(m_globalInfo.mainWindow));
}

void CameraComponentWidget::onChangeEntityCamera(EntityID entityID, const Camera& camera) {
	QObject* senderWidget = sender();
	if (senderWidget != this) {
		if (entityID == m_globalInfo.currentEntityID) {
			updateWidgets(camera);
		}
	}

	SaveTitleChanger::change(reinterpret_cast<MainWindow*>(m_globalInfo.mainWindow));
}

void CameraComponentWidget::onVec3Changed(const nml::vec3& value) {
	Camera newCamera = m_globalInfo.entities[m_globalInfo.currentEntityID].camera.value();

	QObject* senderWidget = sender();
	if (senderWidget == forwardWidget) {
		newCamera.forward = value;
	}
	else if (senderWidget == upWidget) {
		newCamera.up = value;
	}
	m_globalInfo.undoStack->push(new ChangeEntityComponentCommand(m_globalInfo, m_globalInfo.currentEntityID, "Camera", &newCamera));
}

void CameraComponentWidget::onScalarChanged(float value) {
	Camera newCamera = m_globalInfo.entities[m_globalInfo.currentEntityID].camera.value();

	QObject* senderWidget = sender();
	if (senderWidget == fovWidget) {
		newCamera.fov = value;
	}
	else if (senderWidget == nearPlaneWidget) {
		newCamera.nearPlane = value;
	}
	else if (senderWidget == farPlaneWidget) {
		newCamera.farPlane = value;
	}
	m_globalInfo.undoStack->push(new ChangeEntityComponentCommand(m_globalInfo, m_globalInfo.currentEntityID, "Camera", &newCamera));
}
