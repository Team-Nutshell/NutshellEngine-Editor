#include "view_menu.h"
#include <QKeySequence>

ViewMenu::ViewMenu(GlobalInfo& globalInfo): QMenu("&View"), m_globalInfo(globalInfo) {
	m_toggleCurrentEntityVisibilityAction = addAction("Hide Current Entity", this, &ViewMenu::toggleCurrentEntityVisibility);
	m_toggleCurrentEntityVisibilityAction->setShortcut(m_globalInfo.editorParameters.renderer.toggleCurrentEntityVisibility);
	m_toggleCurrentEntityVisibilityAction->setEnabled(false);
	m_toggleGridVisibilityAction = addAction("Hide Grid", this, &ViewMenu::toggleGridVisibility);
	m_toggleGridVisibilityAction->setShortcut(m_globalInfo.editorParameters.renderer.toggleGridVisibility);
	m_toggleBackfaceCullingAction = addAction("Enable Backface Culling", this, &ViewMenu::toggleBackfaceCulling);
	m_toggleBackfaceCullingAction->setShortcut(m_globalInfo.editorParameters.renderer.toggleBackfaceCulling);
	m_toggleCamerasVisibilityAction = addAction("Show Cameras", this, &ViewMenu::toggleCamerasVisibility);
	m_toggleCamerasVisibilityAction->setShortcut(m_globalInfo.editorParameters.renderer.toggleCamerasVisibility);
	m_toggleLightingAction = addAction("Enable Lighting", this, &ViewMenu::toggleLighting);
	m_toggleLightingAction->setShortcut(m_globalInfo.editorParameters.renderer.toggleLighting);
	m_toggleCollidersVisibilityAction = addAction("Show Colliders", this, &ViewMenu::toggleCollidersVisibility);
	m_toggleCollidersVisibilityAction->setShortcut(m_globalInfo.editorParameters.renderer.toggleCollidersVisibility);
	addSeparator();
	m_switchCameraProjectionAction = addAction("Switch Camera Projection to Orthographic", this, &ViewMenu::switchCameraProjection);
	m_switchCameraProjectionAction->setShortcut(m_globalInfo.editorParameters.renderer.switchCameraProjection);
	m_resetCameraAction = addAction("Reset Camera", this, &ViewMenu::resetCamera);
	m_resetCameraAction->setShortcut(m_globalInfo.editorParameters.renderer.resetCamera);
	m_orthographicCameraToXMAction = addAction("Orthographic Camera X-", this, &ViewMenu::orthographicCameraToXM);
	m_orthographicCameraToXMAction->setShortcut(m_globalInfo.editorParameters.renderer.orthographicCameraToXM);
	m_orthographicCameraToXPAction = addAction("Orthographic Camera X+", this, &ViewMenu::orthographicCameraToXP);
	m_orthographicCameraToXPAction->setShortcut(m_globalInfo.editorParameters.renderer.orthographicCameraToXP);
	m_orthographicCameraToYMAction = addAction("Orthographic Camera Y-", this, &ViewMenu::orthographicCameraToYM);
	m_orthographicCameraToYMAction->setShortcut(m_globalInfo.editorParameters.renderer.orthographicCameraToYM);
	m_orthographicCameraToYPAction = addAction("Orthographic Camera Y+", this, &ViewMenu::orthographicCameraToYP);
	m_orthographicCameraToYPAction->setShortcut(m_globalInfo.editorParameters.renderer.orthographicCameraToYP);
	m_orthographicCameraToZMAction = addAction("Orthographic Camera Z-", this, &ViewMenu::orthographicCameraToZM);
	m_orthographicCameraToZMAction->setShortcut(m_globalInfo.editorParameters.renderer.orthographicCameraToZM);
	m_orthographicCameraToZPAction = addAction("Orthographic Camera Z+", this, &ViewMenu::orthographicCameraToZP);
	m_orthographicCameraToZPAction->setShortcut(m_globalInfo.editorParameters.renderer.orthographicCameraToZP);

	connect(&m_globalInfo.signalEmitter, &SignalEmitter::selectEntitySignal, this, &ViewMenu::onSelectEntity);
	connect(&m_globalInfo.signalEmitter, &SignalEmitter::toggleCurrentEntityVisibilitySignal, this, &ViewMenu::onCurrentEntityVisibilityToggled);
	connect(&m_globalInfo.signalEmitter, &SignalEmitter::toggleGridVisibilitySignal, this, &ViewMenu::onGridVisibilityToggled);
	connect(&m_globalInfo.signalEmitter, &SignalEmitter::toggleBackfaceCullingSignal, this, &ViewMenu::onBackfaceCullingToggled);
	connect(&m_globalInfo.signalEmitter, &SignalEmitter::toggleCamerasVisibilitySignal, this, &ViewMenu::onCamerasVisibilityToggled);
	connect(&m_globalInfo.signalEmitter, &SignalEmitter::toggleLightingSignal, this, &ViewMenu::onLightingToggled);
	connect(&m_globalInfo.signalEmitter, &SignalEmitter::toggleCollidersVisibilitySignal, this, &ViewMenu::onCollidersVisibilityToggled);
	connect(&m_globalInfo.signalEmitter, &SignalEmitter::switchCameraProjectionSignal, this, &ViewMenu::onCameraProjectionSwitched);
}

void ViewMenu::toggleCurrentEntityVisibility() {
	m_globalInfo.entities[m_globalInfo.currentEntityID].isVisible = !m_globalInfo.entities[m_globalInfo.currentEntityID].isVisible;
	emit m_globalInfo.signalEmitter.toggleCurrentEntityVisibilitySignal(m_globalInfo.entities[m_globalInfo.currentEntityID].isVisible);
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

void ViewMenu::onSelectEntity() {
	if (m_globalInfo.currentEntityID != NO_ENTITY) {
		m_toggleCurrentEntityVisibilityAction->setEnabled(true);
	}
	else {
		m_toggleCurrentEntityVisibilityAction->setEnabled(false);
	}
}

void ViewMenu::onCurrentEntityVisibilityToggled(bool isEntityVisible) {
	m_globalInfo.entities[m_globalInfo.currentEntityID].isVisible = isEntityVisible;
	m_toggleCurrentEntityVisibilityAction->setText(isEntityVisible ? "Hide Current Entity" : "Show Current Entity");
}

void ViewMenu::onGridVisibilityToggled(bool showGrid) {
	m_showGrid = showGrid;
	m_toggleGridVisibilityAction->setText(m_showGrid ? "Hide Grid" : "Show Grid");
}

void ViewMenu::onBackfaceCullingToggled(bool backfaceCulling) {
	m_backfaceCullingEnabled = backfaceCulling;
	m_toggleBackfaceCullingAction->setText(m_backfaceCullingEnabled ? "Disable Backface Culling" : "Enable Backface Culling");
}

void ViewMenu::onCamerasVisibilityToggled(bool showCameras) {
	m_showCameras = showCameras;
	m_toggleCamerasVisibilityAction->setText(m_showCameras ? "Hide Cameras" : "Show Cameras");
}

void ViewMenu::onLightingToggled(bool lightingEnabled) {
	m_lightingEnabled = lightingEnabled;
	m_toggleLightingAction->setText(m_lightingEnabled ? "Disable Lighting" : "Enable Lighting");
}

void ViewMenu::onCollidersVisibilityToggled(bool showColliders) {
	m_showColliders = showColliders;
	m_toggleCollidersVisibilityAction->setText(m_showColliders ? "Hide Colliders" : "Show Colliders");
}

void ViewMenu::onCameraProjectionSwitched(bool cameraProjectionOrthographic) {
	m_cameraProjectionOrthographic = cameraProjectionOrthographic;
	m_switchCameraProjectionAction->setText(m_cameraProjectionOrthographic ? "Switch Camera Projection to Perspective" : "Switch Camera Projection to Orthographic");
}