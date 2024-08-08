#include "view_menu.h"
#include <QKeySequence>

ViewMenu::ViewMenu(GlobalInfo& globalInfo): QMenu("&View"), m_globalInfo(globalInfo) {
	toggleCurrentEntityVisibilityAction = addAction("Toggle Entity Visibility", this, &ViewMenu::toggleCurrentEntityVisibility);
	toggleCurrentEntityVisibilityAction->setShortcut(m_globalInfo.editorParameters.renderer.toggleCurrentEntityVisibilityKey);
	toggleCurrentEntityVisibilityAction->setEnabled(false);
	toggleGridVisibilityAction = addAction("Hide Grid", this, &ViewMenu::toggleGridVisibility);
	toggleGridVisibilityAction->setShortcut(m_globalInfo.editorParameters.renderer.toggleGridVisibilityKey);
	toggleBackfaceCullingAction = addAction("Enable Backface Culling", this, &ViewMenu::toggleBackfaceCulling);
	toggleBackfaceCullingAction->setShortcut(m_globalInfo.editorParameters.renderer.toggleBackfaceCullingKey);
	toggleCamerasVisibilityAction = addAction("Show Cameras", this, &ViewMenu::toggleCamerasVisibility);
	toggleCamerasVisibilityAction->setShortcut(m_globalInfo.editorParameters.renderer.toggleCamerasVisibilityKey);
	toggleLightingAction = addAction("Enable Lighting", this, &ViewMenu::toggleLighting);
	toggleLightingAction->setShortcut(m_globalInfo.editorParameters.renderer.toggleLightingKey);
	toggleCollidersVisibilityAction = addAction("Show Colliders", this, &ViewMenu::toggleCollidersVisibility);
	toggleCollidersVisibilityAction->setShortcut(m_globalInfo.editorParameters.renderer.toggleCollidersVisibilityKey);
	addSeparator();
	switchCameraProjectionAction = addAction("Switch Camera Projection to Orthographic", this, &ViewMenu::switchCameraProjection);
	switchCameraProjectionAction->setShortcut(m_globalInfo.editorParameters.renderer.switchCameraProjectionKey);
	resetCameraAction = addAction("Reset Camera", this, &ViewMenu::resetCamera);
	resetCameraAction->setShortcut(m_globalInfo.editorParameters.renderer.resetCameraKey);
	orthographicCameraToXMAction = addAction("Orthographic Camera X-", this, &ViewMenu::orthographicCameraToXM);
	orthographicCameraToXMAction->setShortcut(m_globalInfo.editorParameters.renderer.orthographicCameraToXMKey);
	orthographicCameraToXPAction = addAction("Orthographic Camera X+", this, &ViewMenu::orthographicCameraToXP);
	orthographicCameraToXPAction->setShortcut(m_globalInfo.editorParameters.renderer.orthographicCameraToXPKey);
	orthographicCameraToYMAction = addAction("Orthographic Camera Y-", this, &ViewMenu::orthographicCameraToYM);
	orthographicCameraToYMAction->setShortcut(m_globalInfo.editorParameters.renderer.orthographicCameraToYMKey);
	orthographicCameraToYPAction = addAction("Orthographic Camera Y+", this, &ViewMenu::orthographicCameraToYP);
	orthographicCameraToYPAction->setShortcut(m_globalInfo.editorParameters.renderer.orthographicCameraToYPKey);
	orthographicCameraToZMAction = addAction("Orthographic Camera Z-", this, &ViewMenu::orthographicCameraToZM);
	orthographicCameraToZMAction->setShortcut(m_globalInfo.editorParameters.renderer.orthographicCameraToZMKey);
	orthographicCameraToZPAction = addAction("Orthographic Camera Z+", this, &ViewMenu::orthographicCameraToZP);
	orthographicCameraToZPAction->setShortcut(m_globalInfo.editorParameters.renderer.orthographicCameraToZPKey);

	connect(&m_globalInfo.signalEmitter, &SignalEmitter::selectEntitySignal, this, &ViewMenu::onEntitySelected);
	connect(&m_globalInfo.signalEmitter, &SignalEmitter::toggleEntityVisibilitySignal, this, &ViewMenu::onEntityVisibilityToggled);
	connect(&m_globalInfo.signalEmitter, &SignalEmitter::toggleGridVisibilitySignal, this, &ViewMenu::onGridVisibilityToggled);
	connect(&m_globalInfo.signalEmitter, &SignalEmitter::toggleBackfaceCullingSignal, this, &ViewMenu::onBackfaceCullingToggled);
	connect(&m_globalInfo.signalEmitter, &SignalEmitter::toggleCamerasVisibilitySignal, this, &ViewMenu::onCamerasVisibilityToggled);
	connect(&m_globalInfo.signalEmitter, &SignalEmitter::toggleLightingSignal, this, &ViewMenu::onLightingToggled);
	connect(&m_globalInfo.signalEmitter, &SignalEmitter::toggleCollidersVisibilitySignal, this, &ViewMenu::onCollidersVisibilityToggled);
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
	emit m_globalInfo.signalEmitter.toggleGridVisibilitySignal(!m_showGrid);
}

void ViewMenu::toggleBackfaceCulling() {
	emit m_globalInfo.signalEmitter.toggleBackfaceCullingSignal(!m_backfaceCullingEnabled);
}

void ViewMenu::toggleCamerasVisibility() {
	emit m_globalInfo.signalEmitter.toggleCamerasVisibilitySignal(!m_showCameras);
}

void ViewMenu::toggleLighting() {
	emit m_globalInfo.signalEmitter.toggleLightingSignal(!m_lightingEnabled);
}

void ViewMenu::toggleCollidersVisibility() {
	emit m_globalInfo.signalEmitter.toggleCollidersVisibilitySignal(!m_showColliders);
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

void ViewMenu::onEntitySelected() {
	if (m_globalInfo.currentEntityID != NO_ENTITY) {
		toggleCurrentEntityVisibilityAction->setEnabled(true);
		if (m_globalInfo.otherSelectedEntityIDs.empty()) {
			toggleCurrentEntityVisibilityAction->setText(m_globalInfo.entities[m_globalInfo.currentEntityID].isVisible ? "Hide Current Entity" : "Show Current Entity");
		}
		else {
			toggleCurrentEntityVisibilityAction->setText("Toggle Entities Visibility");
		}
	}
	else {
		toggleCurrentEntityVisibilityAction->setEnabled(false);
		toggleCurrentEntityVisibilityAction->setText("Toggle Entity Visibility");
	}
}

void ViewMenu::onEntityVisibilityToggled(EntityID entityID, bool isEntityVisible) {
	m_globalInfo.entities[entityID].isVisible = isEntityVisible;
	if (m_globalInfo.otherSelectedEntityIDs.empty()) {
		toggleCurrentEntityVisibilityAction->setText(isEntityVisible ? "Hide Current Entity" : "Show Current Entity");
	}
}

void ViewMenu::onGridVisibilityToggled(bool showGrid) {
	m_showGrid = showGrid;
	toggleGridVisibilityAction->setText(m_showGrid ? "Hide Grid" : "Show Grid");
}

void ViewMenu::onBackfaceCullingToggled(bool backfaceCulling) {
	m_backfaceCullingEnabled = backfaceCulling;
	toggleBackfaceCullingAction->setText(m_backfaceCullingEnabled ? "Disable Backface Culling" : "Enable Backface Culling");
}

void ViewMenu::onCamerasVisibilityToggled(bool showCameras) {
	m_showCameras = showCameras;
	toggleCamerasVisibilityAction->setText(m_showCameras ? "Hide Cameras" : "Show Cameras");
}

void ViewMenu::onLightingToggled(bool lightingEnabled) {
	m_lightingEnabled = lightingEnabled;
	toggleLightingAction->setText(m_lightingEnabled ? "Disable Lighting" : "Enable Lighting");
}

void ViewMenu::onCollidersVisibilityToggled(bool showColliders) {
	m_showColliders = showColliders;
	toggleCollidersVisibilityAction->setText(m_showColliders ? "Hide Colliders" : "Show Colliders");
}

void ViewMenu::onCameraProjectionSwitched(bool cameraProjectionOrthographic) {
	m_cameraProjectionOrthographic = cameraProjectionOrthographic;
	switchCameraProjectionAction->setText(m_cameraProjectionOrthographic ? "Switch Camera Projection to Perspective" : "Switch Camera Projection to Orthographic");
}