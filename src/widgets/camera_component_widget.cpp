#include "camera_component_widget.h"
#include "component_title_widget.h"
#include "separator_line.h"
#include "main_window.h"
#include "../common/save_title_changer.h"
#include "../undo_commands/change_entities_component_command.h"
#include <QVBoxLayout>
#include <vector>
#include <string>

CameraComponentWidget::CameraComponentWidget(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setLayout(new QVBoxLayout());
	layout()->setAlignment(Qt::AlignmentFlag::AlignTop);
	layout()->setContentsMargins(0, 0, 0, 0);
	layout()->addWidget(new ComponentTitleWidget(m_globalInfo, m_globalInfo.localization.getString("component_camera")));
	forwardWidget = new Vector3Widget(m_globalInfo, m_globalInfo.localization.getString("component_camera_forward"));
	layout()->addWidget(forwardWidget);
	upWidget = new Vector3Widget(m_globalInfo, m_globalInfo.localization.getString("component_camera_up"));
	layout()->addWidget(upWidget);
	std::vector<std::string> elements = { m_globalInfo.localization.getString("component_camera_projection_type_perspective"), m_globalInfo.localization.getString("component_camera_projection_type_orthographic") };
	projectionTypeWidget = new ComboBoxWidget(m_globalInfo, m_globalInfo.localization.getString("component_camera_projection_type"), elements);
	layout()->addWidget(projectionTypeWidget);
	fovWidget = new ScalarWidget(m_globalInfo, m_globalInfo.localization.getString("component_camera_fov"));
	fovWidget->setMin(0.0f);
	layout()->addWidget(fovWidget);
	leftWidget = new ScalarWidget(m_globalInfo, m_globalInfo.localization.getString("component_camera_left"));
	layout()->addWidget(leftWidget);
	rightWidget = new ScalarWidget(m_globalInfo, m_globalInfo.localization.getString("component_camera_right"));
	layout()->addWidget(rightWidget);
	bottomWidget = new ScalarWidget(m_globalInfo, m_globalInfo.localization.getString("component_camera_bottom"));
	layout()->addWidget(bottomWidget);
	topWidget = new ScalarWidget(m_globalInfo, m_globalInfo.localization.getString("component_camera_top"));
	layout()->addWidget(topWidget);
	nearPlaneWidget = new ScalarWidget(m_globalInfo, m_globalInfo.localization.getString("component_camera_near_plane"));
	layout()->addWidget(nearPlaneWidget);
	farPlaneWidget = new ScalarWidget(m_globalInfo, m_globalInfo.localization.getString("component_camera_far_plane"));
	layout()->addWidget(farPlaneWidget);
	layout()->addWidget(new SeparatorLine(m_globalInfo));

	connect(forwardWidget, &Vector3Widget::valueChanged, this, &CameraComponentWidget::onVec3Changed);
	connect(projectionTypeWidget, &ComboBoxWidget::elementSelected, this, &CameraComponentWidget::onElementChanged);
	connect(upWidget, &Vector3Widget::valueChanged, this, &CameraComponentWidget::onVec3Changed);
	connect(fovWidget, &ScalarWidget::valueChanged, this, &CameraComponentWidget::onScalarChanged);
	connect(leftWidget, &ScalarWidget::valueChanged, this, &CameraComponentWidget::onScalarChanged);
	connect(rightWidget, &ScalarWidget::valueChanged, this, &CameraComponentWidget::onScalarChanged);
	connect(bottomWidget, &ScalarWidget::valueChanged, this, &CameraComponentWidget::onScalarChanged);
	connect(topWidget, &ScalarWidget::valueChanged, this, &CameraComponentWidget::onScalarChanged);
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
	projectionTypeWidget->setElementByText(typeToProjectionType(camera.projectionType));
	fovWidget->setValue(camera.fov);
	if (camera.projectionType == "Perspective") {
		fovWidget->setEnabled(true);
	}
	else {
		fovWidget->setEnabled(false);
	}
	leftWidget->setValue(camera.left);
	if (camera.projectionType == "Orthographic") {
		leftWidget->setEnabled(true);
	}
	else {
		leftWidget->setEnabled(false);
	}
	rightWidget->setValue(camera.right);
	if (camera.projectionType == "Orthographic") {
		rightWidget->setEnabled(true);
	}
	else {
		rightWidget->setEnabled(false);
	}
	bottomWidget->setValue(camera.bottom);
	if (camera.projectionType == "Orthographic") {
		bottomWidget->setEnabled(true);
	}
	else {
		bottomWidget->setEnabled(false);
	}
	topWidget->setValue(camera.top);
	if (camera.projectionType == "Orthographic") {
		topWidget->setEnabled(true);
	}
	else {
		topWidget->setEnabled(false);
	}
	nearPlaneWidget->setValue(camera.nearPlane);
	farPlaneWidget->setValue(camera.farPlane);
}

void CameraComponentWidget::updateComponents(const std::vector<EntityID>& entityIDs, std::vector<Camera>& cameras) {
	std::vector<Component*> componentPointers;
	for (size_t i = 0; i < cameras.size(); i++) {
		componentPointers.push_back(&cameras[i]);
	}

	m_globalInfo.actionUndoStack->push(new ChangeEntitiesComponentCommand(m_globalInfo, entityIDs, "Camera", componentPointers));
}

std::string CameraComponentWidget::projectionTypeToType(const std::string& projectionType) {
	if (projectionType == m_globalInfo.localization.getString("component_camera_projection_type_perspective")) {
		return "Perspective";
	}
	else if (projectionType == m_globalInfo.localization.getString("component_camera_projection_type_orthographic")) {
		return "Orthographic";
	}
	else {
		return "Unknown";
	}
}

std::string CameraComponentWidget::typeToProjectionType(const std::string& type) {
	if (type == "Perspective") {
		return m_globalInfo.localization.getString("component_camera_projection_type_perspective");
	}
	else if (type == "Orthographic") {
		return m_globalInfo.localization.getString("component_camera_projection_type_orthographic");
	}
	else {
		return "Unknown";
	}
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

void CameraComponentWidget::onElementChanged(const std::string& element) {
	QObject* senderWidget = sender();

	std::vector<EntityID> entityIDs;
	std::vector<Camera> newCameras;

	std::set<EntityID> selectedEntityIDs = m_globalInfo.otherSelectedEntityIDs;
	selectedEntityIDs.insert(m_globalInfo.currentEntityID);
	for (EntityID selectedEntityID : selectedEntityIDs) {
		if (m_globalInfo.entities[selectedEntityID].camera) {
			Camera newCamera = m_globalInfo.entities[selectedEntityID].camera.value();

			if (senderWidget == projectionTypeWidget) {
				newCamera.projectionType = projectionTypeToType(element);
			}

			entityIDs.push_back(selectedEntityID);
			newCameras.push_back(newCamera);
		}
	}

	updateComponents(entityIDs, newCameras);
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

	std::set<EntityID> selectedEntityIDs = m_globalInfo.otherSelectedEntityIDs;
	selectedEntityIDs.insert(m_globalInfo.currentEntityID);
	for (EntityID selectedEntityID : selectedEntityIDs) {
		if (m_globalInfo.entities[selectedEntityID].camera) {
			Camera newCamera = m_globalInfo.entities[selectedEntityID].camera.value();

			if (senderWidget == fovWidget) {
				newCamera.fov = value;
			}
			else if (senderWidget == leftWidget) {
				newCamera.left = value;
			}
			else if (senderWidget == rightWidget) {
				newCamera.right = value;
			}
			else if (senderWidget == bottomWidget) {
				newCamera.bottom = value;
			}
			else if (senderWidget == topWidget) {
				newCamera.top = value;
			}
			else if (senderWidget == nearPlaneWidget) {
				newCamera.nearPlane = value;
			}
			else if (senderWidget == farPlaneWidget) {
				newCamera.farPlane = value;
			}

			entityIDs.push_back(selectedEntityID);
			newCameras.push_back(newCamera);
		}
	}

	updateComponents(entityIDs, newCameras);
}
