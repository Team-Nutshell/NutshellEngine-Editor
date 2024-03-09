#include "add_component_button.h"

AddComponentButton::AddComponentButton(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setText("Add Component");
	menu = std::make_unique<AddComponentMenu>(m_globalInfo);

	connect(this, &QPushButton::clicked, this, &AddComponentButton::onClick);
}

void AddComponentButton::onClick() {
	if (!m_globalInfo.entities[m_globalInfo.currentEntityID].camera) {
		menu->addCameraComponentAction->setEnabled(true);
	}
	else {
		menu->addCameraComponentAction->setEnabled(false);
	}

	if (!m_globalInfo.entities[m_globalInfo.currentEntityID].light) {
		menu->addLightComponentAction->setEnabled(true);
	}
	else {
		menu->addLightComponentAction->setEnabled(false);
	}

	if (!m_globalInfo.entities[m_globalInfo.currentEntityID].renderable) {
		menu->addRenderableComponentAction->setEnabled(true);
	}
	else {
		menu->addRenderableComponentAction->setEnabled(false);
	}

	if (!m_globalInfo.entities[m_globalInfo.currentEntityID].rigidbody) {
		menu->addRigidbodyComponentAction->setEnabled(true);
	}
	else {
		menu->addRigidbodyComponentAction->setEnabled(false);
	}

	if (!m_globalInfo.entities[m_globalInfo.currentEntityID].collidable) {
		menu->addCollidableComponentAction->setEnabled(true);
	}
	else {
		menu->addCollidableComponentAction->setEnabled(false);
	}

	if (!m_globalInfo.entities[m_globalInfo.currentEntityID].scriptable) {
		menu->addScriptableComponentAction->setEnabled(true);
	}
	else {
		menu->addScriptableComponentAction->setEnabled(false);
	}
}