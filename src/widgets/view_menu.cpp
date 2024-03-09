#include "view_menu.h"
#include "../../external/nlohmann/json.hpp"
#include <QKeySequence>
#include <fstream>

ViewMenu::ViewMenu(GlobalInfo& globalInfo): QMenu("&View"), m_globalInfo(globalInfo) {
	m_toggleCurrentEntityVisibilityAction = addAction("Hide Current Entity", this, &ViewMenu::toggleCurrentEntityVisibility);
	m_toggleCurrentEntityVisibilityAction->setShortcut(QKeySequence::fromString("V"));
	m_toggleCurrentEntityVisibilityAction->setEnabled(false);
	m_toggleBackfaceCullingAction = addAction("Enable Backface Culling", this, &ViewMenu::toggleBackfaceCulling);
	m_toggleBackfaceCullingAction->setShortcut(QKeySequence::fromString("F"));
	m_toggleCamerasVisibilityAction = addAction("Show Cameras", this, &ViewMenu::toggleCameraVisibility);
	m_toggleCamerasVisibilityAction->setShortcut(QKeySequence::fromString("C"));
	addSeparator();
	m_switchCameraProjectionAction = addAction("Switch Camera Projection to Orthographic", this, &ViewMenu::switchCameraProjection);
	m_switchCameraProjectionAction->setShortcut(QKeySequence::fromString("P"));
	m_resetCameraAction = addAction("Reset Camera", this, &ViewMenu::resetCamera);
	m_resetCameraAction->setShortcut(QKeySequence::fromString("0"));
	m_orthographicCameraToXMAction = addAction("Orthographic Camera X-", this, &ViewMenu::orthographicCameraToXM);
	m_orthographicCameraToXMAction->setShortcut(QKeySequence::fromString("4"));
	m_orthographicCameraToXPAction = addAction("Orthographic Camera X+", this, &ViewMenu::orthographicCameraToXP);
	m_orthographicCameraToXPAction->setShortcut(QKeySequence::fromString("6"));
	m_orthographicCameraToYMAction = addAction("Orthographic Camera Y-", this, &ViewMenu::orthographicCameraToYM);
	m_orthographicCameraToYMAction->setShortcut(QKeySequence::fromString("1"));
	m_orthographicCameraToYPAction = addAction("Orthographic Camera Y+", this, &ViewMenu::orthographicCameraToYP);
	m_orthographicCameraToYPAction->setShortcut(QKeySequence::fromString("7"));
	m_orthographicCameraToZMAction = addAction("Orthographic Camera Z-", this, &ViewMenu::orthographicCameraToZM);
	m_orthographicCameraToZMAction->setShortcut(QKeySequence::fromString("8"));
	m_orthographicCameraToZPAction = addAction("Orthographic Camera Z+", this, &ViewMenu::orthographicCameraToZP);
	m_orthographicCameraToZPAction->setShortcut(QKeySequence::fromString("2"));

	std::fstream optionsFile("assets/options.json", std::ios::in);
	if (optionsFile.is_open()) {
		if (!nlohmann::json::accept(optionsFile)) {
			std::cout << "\"assets/options.json\" is not a valid JSON file." << std::endl;
			return;
		}
	}
	else {
		std::cout << "\"assets/options.json\" cannot be opened." << std::endl;
		return;
	}

	optionsFile = std::fstream("assets/options.json", std::ios::in);
	nlohmann::json j = nlohmann::json::parse(optionsFile);

	if (j.contains("renderer")) {
		if (j["renderer"].contains("toggleCurrentEntityVisibility")) {
			m_toggleCurrentEntityVisibilityAction->setShortcut(QString::fromStdString(j["renderer"]["toggleCurrentEntityVisibility"]));
		}
		if (j["renderer"].contains("toggleBackfaceCulling")) {
			m_toggleBackfaceCullingAction->setShortcut(QString::fromStdString(j["renderer"]["toggleBackfaceCulling"]));
		}
		if (j["renderer"].contains("toggleCamerasVisibility")) {
			m_toggleCamerasVisibilityAction->setShortcut(QString::fromStdString(j["renderer"]["toggleCamerasVisibility"]));
		}
		if (j["renderer"].contains("switchCameraProjection")) {
			m_switchCameraProjectionAction->setShortcut(QString::fromStdString(j["renderer"]["switchCameraProjection"]));
		}
		if (j["renderer"].contains("resetCamera")) {
			m_resetCameraAction->setShortcut(QString::fromStdString(j["renderer"]["resetCamera"]));
		}
		if (j["renderer"].contains("orthographicCameraToXM")) {
			m_orthographicCameraToXMAction->setShortcut(QString::fromStdString(j["renderer"]["orthographicCameraToXM"]));
		}
		if (j["renderer"].contains("orthographicCameraToXP")) {
			m_orthographicCameraToXPAction->setShortcut(QString::fromStdString(j["renderer"]["orthographicCameraToXP"]));
		}
		if (j["renderer"].contains("orthographicCameraToYM")) {
			m_orthographicCameraToYMAction->setShortcut(QString::fromStdString(j["renderer"]["orthographicCameraToYM"]));
		}
		if (j["renderer"].contains("orthographicCameraToYP")) {
			m_orthographicCameraToYPAction->setShortcut(QString::fromStdString(j["renderer"]["orthographicCameraToYP"]));
		}
		if (j["renderer"].contains("orthographicCameraToZM")) {
			m_orthographicCameraToZMAction->setShortcut(QString::fromStdString(j["renderer"]["orthographicCameraToZM"]));
		}
		if (j["renderer"].contains("orthographicCameraToZP")) {
			m_orthographicCameraToZPAction->setShortcut(QString::fromStdString(j["renderer"]["orthographicCameraToZP"]));
		}
	}

	connect(&m_globalInfo.signalEmitter, &SignalEmitter::selectEntitySignal, this, &ViewMenu::onSelectEntity);
	connect(&m_globalInfo.signalEmitter, &SignalEmitter::toggleCurrentEntityVisibilitySignal, this, &ViewMenu::onCurrentEntityVisibilityToggled);
	connect(&m_globalInfo.signalEmitter, &SignalEmitter::toggleBackfaceCullingSignal, this, &ViewMenu::onBackfaceCullingToggled);
	connect(&m_globalInfo.signalEmitter, &SignalEmitter::toggleCamerasVisibilitySignal, this, &ViewMenu::onCameraVisibilityToggled);
	connect(&m_globalInfo.signalEmitter, &SignalEmitter::switchCameraProjectionSignal, this, &ViewMenu::onCameraProjectionSwitched);
}

void ViewMenu::toggleCurrentEntityVisibility() {
	m_globalInfo.entities[m_globalInfo.currentEntityID].isVisible = !m_globalInfo.entities[m_globalInfo.currentEntityID].isVisible;
	emit m_globalInfo.signalEmitter.toggleCurrentEntityVisibilitySignal(m_globalInfo.entities[m_globalInfo.currentEntityID].isVisible);
}

void ViewMenu::toggleBackfaceCulling() {
	emit m_globalInfo.signalEmitter.toggleBackfaceCullingSignal(!m_backfaceCullingEnabled);
}

void ViewMenu::toggleCameraVisibility() {
	emit m_globalInfo.signalEmitter.toggleCamerasVisibilitySignal(!m_showCameras);
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

void ViewMenu::onBackfaceCullingToggled(bool backfaceCulling) {
	m_backfaceCullingEnabled = backfaceCulling;
	m_toggleBackfaceCullingAction->setText(m_backfaceCullingEnabled ? "Disable Backface Culling" : "Enable Backface Culling");
}

void ViewMenu::onCameraVisibilityToggled(bool showCameras) {
	m_showCameras = showCameras;
	m_toggleCamerasVisibilityAction->setText(m_showCameras ? "Hide Cameras" : "Show Cameras");
}

void ViewMenu::onCameraProjectionSwitched(bool cameraProjectionOrthographic) {
	m_cameraProjectionOrthographic = cameraProjectionOrthographic;
	m_switchCameraProjectionAction->setText(m_cameraProjectionOrthographic ? "Switch Camera Projection to Perspective" : "Switch Camera Projection to Orthographic");
}