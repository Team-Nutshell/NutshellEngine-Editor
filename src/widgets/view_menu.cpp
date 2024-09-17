#include "view_menu.h"
#include <QKeySequence>
#include <fstream>

ViewMenu::ViewMenu(GlobalInfo& globalInfo) : QMenu("&" + QString::fromStdString(globalInfo.localization.getString("header_view"))), m_globalInfo(globalInfo) {
	toggleCurrentEntityVisibilityAction = addAction(QString::fromStdString(m_globalInfo.localization.getString("header_view_toggle_entity_visibility")), this, &ViewMenu::toggleCurrentEntityVisibility);
	toggleCurrentEntityVisibilityAction->setShortcut(m_globalInfo.editorParameters.renderer.toggleCurrentEntityVisibilityKey);
	toggleCurrentEntityVisibilityAction->setEnabled(false);
	toggleGridVisibilityAction = addAction(QString::fromStdString(m_globalInfo.localization.getString("header_view_hide_grid")), this, &ViewMenu::toggleGridVisibility);
	toggleGridVisibilityAction->setShortcut(m_globalInfo.editorParameters.renderer.toggleGridVisibilityKey);
	toggleBackfaceCullingAction = addAction(QString::fromStdString(m_globalInfo.localization.getString("header_view_enable_backface_culling")), this, &ViewMenu::toggleBackfaceCulling);
	toggleBackfaceCullingAction->setShortcut(m_globalInfo.editorParameters.renderer.toggleBackfaceCullingKey);
	toggleCamerasVisibilityAction = addAction(QString::fromStdString(m_globalInfo.localization.getString("header_view_show_cameras")), this, &ViewMenu::toggleCamerasVisibility);
	toggleCamerasVisibilityAction->setShortcut(m_globalInfo.editorParameters.renderer.toggleCamerasVisibilityKey);
	toggleLightingAction = addAction(QString::fromStdString(m_globalInfo.localization.getString("header_view_enable_lighting")), this, &ViewMenu::toggleLighting);
	toggleLightingAction->setShortcut(m_globalInfo.editorParameters.renderer.toggleLightingKey);
	toggleCollidersVisibilityAction = addAction(QString::fromStdString(m_globalInfo.localization.getString("header_view_show_colliders")), this, &ViewMenu::toggleCollidersVisibility);
	toggleCollidersVisibilityAction->setShortcut(m_globalInfo.editorParameters.renderer.toggleCollidersVisibilityKey);
	addSeparator();
	switchCameraProjectionAction = addAction(QString::fromStdString(m_globalInfo.localization.getString("header_view_switch_camera_projection_orthographic")), this, &ViewMenu::switchCameraProjection);
	switchCameraProjectionAction->setShortcut(m_globalInfo.editorParameters.renderer.switchCameraProjectionKey);
	resetCameraAction = addAction(QString::fromStdString(m_globalInfo.localization.getString("header_view_reset_camera")), this, &ViewMenu::resetCamera);
	resetCameraAction->setShortcut(m_globalInfo.editorParameters.renderer.resetCameraKey);
	orthographicCameraToXMAction = addAction(QString::fromStdString(m_globalInfo.localization.getString("header_view_camera_orthographic_xm")), this, &ViewMenu::orthographicCameraToXM);
	orthographicCameraToXMAction->setShortcut(m_globalInfo.editorParameters.renderer.orthographicCameraToXMKey);
	orthographicCameraToXPAction = addAction(QString::fromStdString(m_globalInfo.localization.getString("header_view_camera_orthographic_xp")), this, &ViewMenu::orthographicCameraToXP);
	orthographicCameraToXPAction->setShortcut(m_globalInfo.editorParameters.renderer.orthographicCameraToXPKey);
	orthographicCameraToYMAction = addAction(QString::fromStdString(m_globalInfo.localization.getString("header_view_camera_orthographic_ym")), this, &ViewMenu::orthographicCameraToYM);
	orthographicCameraToYMAction->setShortcut(m_globalInfo.editorParameters.renderer.orthographicCameraToYMKey);
	orthographicCameraToYPAction = addAction(QString::fromStdString(m_globalInfo.localization.getString("header_view_camera_orthographic_yp")), this, &ViewMenu::orthographicCameraToYP);
	orthographicCameraToYPAction->setShortcut(m_globalInfo.editorParameters.renderer.orthographicCameraToYPKey);
	orthographicCameraToZMAction = addAction(QString::fromStdString(m_globalInfo.localization.getString("header_view_camera_orthographic_zm")), this, &ViewMenu::orthographicCameraToZM);
	orthographicCameraToZMAction->setShortcut(m_globalInfo.editorParameters.renderer.orthographicCameraToZMKey);
	orthographicCameraToZPAction = addAction(QString::fromStdString(m_globalInfo.localization.getString("header_view_camera_orthographic_zp")), this, &ViewMenu::orthographicCameraToZP);
	orthographicCameraToZPAction->setShortcut(m_globalInfo.editorParameters.renderer.orthographicCameraToZPKey);

	setGridVisibility(m_globalInfo.editorParameters.renderer.showGrid);
	setBackfaceCulling(m_globalInfo.editorParameters.renderer.enableBackfaceCulling);
	setCamerasVisibility(m_globalInfo.editorParameters.renderer.showCameras);
	setLighting(m_globalInfo.editorParameters.renderer.enableLighting);
	setCollidersVisibility(m_globalInfo.editorParameters.renderer.showColliders);

	connect(&m_globalInfo.signalEmitter, &SignalEmitter::selectEntitySignal, this, &ViewMenu::onEntitySelected);
	connect(&m_globalInfo.signalEmitter, &SignalEmitter::toggleEntityVisibilitySignal, this, &ViewMenu::onEntityVisibilityToggled);
	connect(&m_globalInfo.signalEmitter, &SignalEmitter::switchCameraProjectionSignal, this, &ViewMenu::onCameraProjectionSwitched);
}

void ViewMenu::toggleCurrentEntityVisibility() {
	m_globalInfo.entities[m_globalInfo.currentEntityID].isVisible = !m_globalInfo.entities[m_globalInfo.currentEntityID].isVisible;
	emit m_globalInfo.signalEmitter.toggleEntityVisibilitySignal(m_globalInfo.currentEntityID, m_globalInfo.entities[m_globalInfo.currentEntityID].isVisible);
	for (EntityID otherSelectedEntityID : m_globalInfo.otherSelectedEntityIDs) {
		m_globalInfo.entities[otherSelectedEntityID].isVisible = !m_globalInfo.entities[otherSelectedEntityID].isVisible;
		emit m_globalInfo.signalEmitter.toggleEntityVisibilitySignal(otherSelectedEntityID, m_globalInfo.entities[otherSelectedEntityID].isVisible);
	}
}

void ViewMenu::toggleGridVisibility() {
	setGridVisibility(!m_globalInfo.editorParameters.renderer.showGrid);

	save();
}

void ViewMenu::toggleBackfaceCulling() {
	setBackfaceCulling(!m_globalInfo.editorParameters.renderer.enableBackfaceCulling);

	save();
}

void ViewMenu::toggleCamerasVisibility() {
	setCamerasVisibility(!m_globalInfo.editorParameters.renderer.showCameras);

	save();
}

void ViewMenu::toggleLighting() {
	setLighting(!m_globalInfo.editorParameters.renderer.enableLighting);

	save();
}

void ViewMenu::toggleCollidersVisibility() {
	setCollidersVisibility(!m_globalInfo.editorParameters.renderer.showColliders);

	save();
}

void ViewMenu::switchCameraProjection() {
	emit m_globalInfo.signalEmitter.switchCameraProjectionSignal(!m_cameraProjectionOrthographic);
}

void ViewMenu::resetCamera() {
	emit m_globalInfo.signalEmitter.resetCameraSignal();
}

void ViewMenu::orthographicCameraToXM() {
	if (!m_cameraProjectionOrthographic) {
		emit m_globalInfo.signalEmitter.switchCameraProjectionSignal(true);
	}
	emit m_globalInfo.signalEmitter.orthographicCameraToAxisSignal(nml::vec3(-1.0f, 0.0f, 0.0f));
}

void ViewMenu::orthographicCameraToXP() {
	if (!m_cameraProjectionOrthographic) {
		emit m_globalInfo.signalEmitter.switchCameraProjectionSignal(true);
	}
	emit m_globalInfo.signalEmitter.orthographicCameraToAxisSignal(nml::vec3(1.0f, 0.0f, 0.0f));
}

void ViewMenu::orthographicCameraToYM() {
	if (!m_cameraProjectionOrthographic) {
		emit m_globalInfo.signalEmitter.switchCameraProjectionSignal(true);
	}
	emit m_globalInfo.signalEmitter.orthographicCameraToAxisSignal(nml::vec3(0.0f, -1.0f, 0.0f));
}

void ViewMenu::orthographicCameraToYP() {
	if (!m_cameraProjectionOrthographic) {
		emit m_globalInfo.signalEmitter.switchCameraProjectionSignal(true);
	}
	emit m_globalInfo.signalEmitter.orthographicCameraToAxisSignal(nml::vec3(0.0f, 1.0f, 0.0f));
}

void ViewMenu::orthographicCameraToZM() {
	if (!m_cameraProjectionOrthographic) {
		emit m_globalInfo.signalEmitter.switchCameraProjectionSignal(true);
	}
	emit m_globalInfo.signalEmitter.orthographicCameraToAxisSignal(nml::vec3(0.0f, 0.0f, -1.0f));
}

void ViewMenu::orthographicCameraToZP() {
	if (!m_cameraProjectionOrthographic) {
		emit m_globalInfo.signalEmitter.switchCameraProjectionSignal(true);
	}
	emit m_globalInfo.signalEmitter.orthographicCameraToAxisSignal(nml::vec3(0.0f, 0.0f, 1.0f));
}

void ViewMenu::save() {
	nlohmann::json j = m_globalInfo.editorParameters.toJson();

	std::fstream optionsFile("options.json", std::ios::out | std::ios::trunc);
	if (j.empty()) {
		optionsFile << "{\n}";
	}
	else {
		optionsFile << j.dump(1, '\t');
	}
}

void ViewMenu::onEntitySelected() {
	if (m_globalInfo.currentEntityID != NO_ENTITY) {
		toggleCurrentEntityVisibilityAction->setEnabled(true);
		if (m_globalInfo.otherSelectedEntityIDs.empty()) {
			toggleCurrentEntityVisibilityAction->setText(m_globalInfo.entities[m_globalInfo.currentEntityID].isVisible ? QString::fromStdString(m_globalInfo.localization.getString("header_view_hide_current_entity")) : QString::fromStdString(m_globalInfo.localization.getString("header_view_show_current_entity")));
		}
		else {
			toggleCurrentEntityVisibilityAction->setText(QString::fromStdString(m_globalInfo.localization.getString("header_view_toggle_entities_visibility")));
		}
	}
	else {
		toggleCurrentEntityVisibilityAction->setEnabled(false);
		toggleCurrentEntityVisibilityAction->setText(QString::fromStdString(m_globalInfo.localization.getString("header_view_toggle_entity_visibility")));
	}
}

void ViewMenu::onEntityVisibilityToggled(EntityID entityID, bool isEntityVisible) {
	m_globalInfo.entities[entityID].isVisible = isEntityVisible;
	if (m_globalInfo.otherSelectedEntityIDs.empty()) {
		toggleCurrentEntityVisibilityAction->setText(isEntityVisible ? QString::fromStdString(m_globalInfo.localization.getString("header_view_hide_current_entity")) : QString::fromStdString(m_globalInfo.localization.getString("header_view_show_current_entity")));
	}
}

void ViewMenu::setGridVisibility(bool showGrid) {
	m_globalInfo.editorParameters.renderer.showGrid = showGrid;
	toggleGridVisibilityAction->setText(m_globalInfo.editorParameters.renderer.showGrid ? QString::fromStdString(m_globalInfo.localization.getString("header_view_hide_grid")) : QString::fromStdString(m_globalInfo.localization.getString("header_view_show_grid")));
}

void ViewMenu::setBackfaceCulling(bool backfaceCulling) {
	m_globalInfo.editorParameters.renderer.enableBackfaceCulling = backfaceCulling;
	toggleBackfaceCullingAction->setText(m_globalInfo.editorParameters.renderer.enableBackfaceCulling ? QString::fromStdString(m_globalInfo.localization.getString("header_view_disable_backface_culling")) : QString::fromStdString(m_globalInfo.localization.getString("header_view_enable_backface_culling")));
}

void ViewMenu::setCamerasVisibility(bool showCameras) {
	m_globalInfo.editorParameters.renderer.showCameras = showCameras;
	toggleCamerasVisibilityAction->setText(m_globalInfo.editorParameters.renderer.showCameras ? QString::fromStdString(m_globalInfo.localization.getString("header_view_hide_cameras")) : QString::fromStdString(m_globalInfo.localization.getString("header_view_show_cameras")));
}

void ViewMenu::setLighting(bool lightingEnabled) {
	m_globalInfo.editorParameters.renderer.enableLighting = lightingEnabled;
	toggleLightingAction->setText(m_globalInfo.editorParameters.renderer.enableLighting ? QString::fromStdString(m_globalInfo.localization.getString("header_view_disable_lighting")) : QString::fromStdString(m_globalInfo.localization.getString("header_view_enable_lighting")));
}

void ViewMenu::setCollidersVisibility(bool showColliders) {
	m_globalInfo.editorParameters.renderer.showColliders = showColliders;
	toggleCollidersVisibilityAction->setText(m_globalInfo.editorParameters.renderer.showColliders ? QString::fromStdString(m_globalInfo.localization.getString("header_view_hide_colliders")) : QString::fromStdString(m_globalInfo.localization.getString("header_view_show_colliders")));
}

void ViewMenu::onCameraProjectionSwitched(bool cameraProjectionOrthographic) {
	m_cameraProjectionOrthographic = cameraProjectionOrthographic;
	switchCameraProjectionAction->setText(m_cameraProjectionOrthographic ? QString::fromStdString(m_globalInfo.localization.getString("header_view_switch_camera_projection_perspective")) : QString::fromStdString(m_globalInfo.localization.getString("header_view_switch_camera_projection_orthographic")));
}