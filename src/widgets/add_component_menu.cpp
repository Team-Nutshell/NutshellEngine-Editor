#include "add_component_menu.h"
#include "../undo_commands/add_entity_component_command.h"

AddComponentMenu::AddComponentMenu(GlobalInfo& globalInfo): QMenu("&File"), m_globalInfo(globalInfo) {
	addCameraComponentAction = addAction("Add Camera Component", this, &AddComponentMenu::addCameraComponent);
	addLightComponentAction = addAction("Add Light Component", this, &AddComponentMenu::addLightComponent);
	addRenderableComponentAction = addAction("Add Renderable Component", this, &AddComponentMenu::addRenderableComponent);
	addRigidbodyComponentAction = addAction("Add Rigidbody Component", this, &AddComponentMenu::addRigidbodyComponent);
	addCollidableComponentAction = addAction("Add Collidable Component", this, &AddComponentMenu::addCollidableComponent);
	addScriptableComponentAction = addAction("Add Scriptable Component", this, &AddComponentMenu::addScriptableComponent);
}

void AddComponentMenu::addCameraComponent() {
	m_globalInfo.undoStack->push(new AddEntityComponentCommand(m_globalInfo, m_globalInfo.currentEntityID, "Camera"));
}

void AddComponentMenu::addLightComponent() {
	m_globalInfo.undoStack->push(new AddEntityComponentCommand(m_globalInfo, m_globalInfo.currentEntityID, "Light"));
}

void AddComponentMenu::addRenderableComponent() {
	m_globalInfo.undoStack->push(new AddEntityComponentCommand(m_globalInfo, m_globalInfo.currentEntityID, "Renderable"));
}

void AddComponentMenu::addRigidbodyComponent() {
	m_globalInfo.undoStack->push(new AddEntityComponentCommand(m_globalInfo, m_globalInfo.currentEntityID, "Rigidbody"));
}

void AddComponentMenu::addCollidableComponent() {
	m_globalInfo.undoStack->push(new AddEntityComponentCommand(m_globalInfo, m_globalInfo.currentEntityID, "Collidable"));
}

void AddComponentMenu::addScriptableComponent() {
	m_globalInfo.undoStack->push(new AddEntityComponentCommand(m_globalInfo, m_globalInfo.currentEntityID, "Scriptable"));
}
