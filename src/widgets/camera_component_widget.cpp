#include "camera_component_widget.h"
#include "component_title_widget.h"
#include "separator_line.h"
#include "main_window.h"
#include "../common/save_title_changer.h"
#include "../undo_commands/change_entities_component_command.h"
#include <QVBoxLayout>

CameraComponentWidget::CameraComponentWidget(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setLayout(new QVBoxLayout());
	layout()->setAlignment(Qt::AlignmentFlag::AlignTop);
	layout()->setContentsMargins(0, 0, 0, 0);
	layout()->addWidget(new ComponentTitleWidget(m_globalInfo, m_globalInfo.localization.getString("component_camera")));
	forwardWidget = new Vector3Widget(m_globalInfo, m_globalInfo.localization.getString("component_camera_forward"));
	layout()->addWidget(forwardWidget);
	upWidget = new Vector3Widget(m_globalInfo, m_globalInfo.localization.getString("component_camera_up"));
	layout()->addWidget(upWidget);
	fovWidget = new ScalarWidget(m_globalInfo, m_globalInfo.localization.getString("component_camera_fov"));
	fovWidget->setMin(0.0f);
	layout()->addWidget(fovWidget);
	nearPlaneWidget = new ScalarWidget(m_globalInfo, m_globalInfo.localization.getString("component_camera_near_plane"));
	layout()->addWidget(nearPlaneWidget);
	farPlaneWidget = new ScalarWidget(m_globalInfo, m_globalInfo.localization.getString("component_camera_far_plane"));
	layout()->addWidget(farPlaneWidget);
	layout()->addWidget(new SeparatorLine(m_globalInfo));

	connect(forwardWidget, &Vector3Widget::valueChanged, this, &CameraComponentWidget::onVec3Changed);
	connect(upWidget, &Vector3Widget::valueChanged, this, &CameraComponentWidget::onVec3Changed);
	connect(fovWidget, &ScalarWidget::valueChanged, this, &CameraComponentWidget::onScalarChanged);
	connect(nearPlaneWidget, &ScalarWidget::valueChanged, this, &CameraComponentWidget::onScalarChanged);
	connect(farPlaneWidget, &ScalarWidget::valueChanged, this, &CameraComponentWidget::onScalarChanged);
	connect(&globalInfo.signalEmitter, &SignalEmitter::selectEntitySignal, this, &CameraComponentWidget::onEntitySelected);
	connect(&globalInfo.signalEmitter, &SignalEmitter::addEntityCameraSignal, this, &CameraComponentWidget::onEntityCameraAdded);
	connect(&globalInfo.signalEmitter, &SignalEmitter::removeEntityCameraSignal, this, &CameraComponentWidget::onEntityCameraRemoved);
	connect(&globalInfo.signalEmitter, &SignalEmitter::changeEntityCameraSignal, this, &CameraComponentWidget::onEntityCameraChanged);
}

void CameraComponentWidget::updateWidgets(const Camera& camera) {
	forwardWidget->setValue(camera.forward);
	upWidget->setValue(camera.up);
	fovWidget->setValue(camera.fov);
	nearPlaneWidget->setValue(camera.nearPlane);
	farPlaneWidget->setValue(camera.farPlane);
}

void CameraComponentWidget::updateComponents(const std::vector<EntityID>& entityIDs, std::vector<Camera>& cameras) {
	std::vector<Component*> componentPointers;
	for (size_t i = 0; i < cameras.size(); i++) {
		componentPointers.push_back(&cameras[i]);
	}

	m_globalInfo.undoStack->push(new ChangeEntitiesComponentCommand(m_globalInfo, entityIDs, "Camera", componentPointers));
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

void CameraComponentWidget::onEntityCameraAdded(EntityID entityID) {
	if (entityID == m_globalInfo.currentEntityID) {
		const Camera& camera = m_globalInfo.entities[m_globalInfo.currentEntityID].camera.value();
		updateWidgets(camera);
		show();
	}

	SaveTitleChanger::change(m_globalInfo.mainWindow);
}

void CameraComponentWidget::onEntityCameraRemoved(EntityID entityID) {
	if (entityID == m_globalInfo.currentEntityID) {
		hide();
	}

	SaveTitleChanger::change(m_globalInfo.mainWindow);
}

void CameraComponentWidget::onEntityCameraChanged(EntityID entityID, const Camera& camera) {
	QObject* senderWidget = sender();
	if (senderWidget != this) {
		if (entityID == m_globalInfo.currentEntityID) {
			updateWidgets(camera);
		}
	}

	SaveTitleChanger::change(m_globalInfo.mainWindow);
}

void CameraComponentWidget::onVec3Changed(const nml::vec3& value) {
	QObject* senderWidget = sender();

	std::vector<EntityID> entityIDs;
	std::vector<Camera> newCameras;

	Camera newCamera = m_globalInfo.entities[m_globalInfo.currentEntityID].camera.value();

	uint8_t changedIndex = 255;
	if (senderWidget == forwardWidget) {
		for (uint8_t i = 0; i < 3; i++) {
			if (newCamera.forward[i] != value[i]) {
				changedIndex = i;
				break;
			}
		}
		newCamera.forward = value;
	}
	else if (senderWidget == upWidget) {
		for (uint8_t i = 0; i < 3; i++) {
			if (newCamera.up[i] != value[i]) {
				changedIndex = i;
				break;
			}
		}
		newCamera.up = value;
	}
	entityIDs.push_back(m_globalInfo.currentEntityID);
	newCameras.push_back(newCamera);

	for (EntityID otherSelectedEntityID : m_globalInfo.otherSelectedEntityIDs) {
		if (m_globalInfo.entities[otherSelectedEntityID].camera) {
			newCamera = m_globalInfo.entities[otherSelectedEntityID].camera.value();

			if (senderWidget == forwardWidget) {
				newCamera.forward[changedIndex] = value[changedIndex];
			}
			else if (senderWidget == upWidget) {
				newCamera.up[changedIndex] = value[changedIndex];
			}

			entityIDs.push_back(otherSelectedEntityID);
			newCameras.push_back(newCamera);
		}
	}

	updateComponents(entityIDs, newCameras);
}

void CameraComponentWidget::onScalarChanged(float value) {
	QObject* senderWidget = sender();

	std::vector<EntityID> entityIDs;
	std::vector<Camera> newCameras;

	Camera newCamera = m_globalInfo.entities[m_globalInfo.currentEntityID].camera.value();

	if (senderWidget == fovWidget) {
		newCamera.fov = value;
	}
	else if (senderWidget == nearPlaneWidget) {
		newCamera.nearPlane = value;
	}
	else if (senderWidget == farPlaneWidget) {
		newCamera.farPlane = value;
	}
	entityIDs.push_back(m_globalInfo.currentEntityID);
	newCameras.push_back(newCamera);

	for (EntityID otherSelectedEntityID : m_globalInfo.otherSelectedEntityIDs) {
		if (m_globalInfo.entities[otherSelectedEntityID].camera) {
			newCamera = m_globalInfo.entities[otherSelectedEntityID].camera.value();

			if (senderWidget == fovWidget) {
				newCamera.fov = value;
			}
			else if (senderWidget == nearPlaneWidget) {
				newCamera.nearPlane = value;
			}
			else if (senderWidget == farPlaneWidget) {
				newCamera.farPlane = value;
			}

			entityIDs.push_back(otherSelectedEntityID);
			newCameras.push_back(newCamera);
		}
	}

	updateComponents(entityIDs, newCameras);
}
