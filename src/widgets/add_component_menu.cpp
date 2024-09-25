#include "add_component_menu.h"
#include "../undo_commands/add_entity_component_command.h"

AddComponentMenu::AddComponentMenu(GlobalInfo& globalInfo) : QMenu("&File"), m_globalInfo(globalInfo) {
	addCameraComponentAction = addAction(QString::fromStdString(m_globalInfo.localization.getString("entity_add_component_camera")), this, &AddComponentMenu::addCameraComponent);
	addLightComponentAction = addAction(QString::fromStdString(m_globalInfo.localization.getString("entity_add_component_light")), this, &AddComponentMenu::addLightComponent);
	addRenderableComponentAction = addAction(QString::fromStdString(m_globalInfo.localization.getString("entity_add_component_renderable")), this, &AddComponentMenu::addRenderableComponent);
	addRigidbodyComponentAction = addAction(QString::fromStdString(m_globalInfo.localization.getString("entity_add_component_rigidbody")), this, &AddComponentMenu::addRigidbodyComponent);
	addCollidableComponentAction = addAction(QString::fromStdString(m_globalInfo.localization.getString("entity_add_component_collidable")), this, &AddComponentMenu::addCollidableComponent);
	addSoundListenerComponentAction = addAction(QString::fromStdString(m_globalInfo.localization.getString("entity_add_component_sound_listener")), this, &AddComponentMenu::addSoundListenerComponent);
	addScriptableComponentAction = addAction(QString::fromStdString(m_globalInfo.localization.getString("entity_add_component_scriptable")), this, &AddComponentMenu::addScriptableComponent);
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

void AddComponentMenu::addSoundListenerComponent() {
	m_globalInfo.undoStack->push(new AddEntityComponentCommand(m_globalInfo, m_globalInfo.currentEntityID, "SoundListener"));
}

void AddComponentMenu::addScriptableComponent() {
	m_globalInfo.undoStack->push(new AddEntityComponentCommand(m_globalInfo, m_globalInfo.currentEntityID, "Scriptable"));
}
