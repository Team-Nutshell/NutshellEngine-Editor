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
	std::set<EntityID> selectedEntityIDs = m_globalInfo.otherSelectedEntityIDs;
	selectedEntityIDs.insert(m_globalInfo.currentEntityID);

	std::vector<EntityID> entityIDs;
	for (EntityID selectedEntityID : selectedEntityIDs) {
		if (!m_globalInfo.entities[selectedEntityID].camera) {
			entityIDs.push_back(selectedEntityID);
		}
	}

	m_globalInfo.actionUndoStack->push(new AddEntityComponentCommand(m_globalInfo, entityIDs, "Camera"));
}

void AddComponentMenu::addLightComponent() {
	std::set<EntityID> selectedEntityIDs = m_globalInfo.otherSelectedEntityIDs;
	selectedEntityIDs.insert(m_globalInfo.currentEntityID);

	std::vector<EntityID> entityIDs;
	for (EntityID selectedEntityID : selectedEntityIDs) {
		if (!m_globalInfo.entities[selectedEntityID].light) {
			entityIDs.push_back(selectedEntityID);
		}
	}

	m_globalInfo.actionUndoStack->push(new AddEntityComponentCommand(m_globalInfo, entityIDs, "Light"));
}

void AddComponentMenu::addRenderableComponent() {
	std::set<EntityID> selectedEntityIDs = m_globalInfo.otherSelectedEntityIDs;
	selectedEntityIDs.insert(m_globalInfo.currentEntityID);

	std::vector<EntityID> entityIDs;
	for (EntityID selectedEntityID : selectedEntityIDs) {
		if (!m_globalInfo.entities[selectedEntityID].renderable) {
			entityIDs.push_back(selectedEntityID);
		}
	}

	m_globalInfo.actionUndoStack->push(new AddEntityComponentCommand(m_globalInfo, entityIDs, "Renderable"));
}

void AddComponentMenu::addRigidbodyComponent() {
	std::set<EntityID> selectedEntityIDs = m_globalInfo.otherSelectedEntityIDs;
	selectedEntityIDs.insert(m_globalInfo.currentEntityID);

	std::vector<EntityID> entityIDs;
	for (EntityID selectedEntityID : selectedEntityIDs) {
		if (!m_globalInfo.entities[selectedEntityID].rigidbody) {
			entityIDs.push_back(selectedEntityID);
		}
	}

	m_globalInfo.actionUndoStack->push(new AddEntityComponentCommand(m_globalInfo, entityIDs, "Rigidbody"));
}

void AddComponentMenu::addCollidableComponent() {
	std::set<EntityID> selectedEntityIDs = m_globalInfo.otherSelectedEntityIDs;
	selectedEntityIDs.insert(m_globalInfo.currentEntityID);

	std::vector<EntityID> entityIDs;
	for (EntityID selectedEntityID : selectedEntityIDs) {
		if (!m_globalInfo.entities[selectedEntityID].collidable) {
			entityIDs.push_back(selectedEntityID);
		}
	}

	m_globalInfo.actionUndoStack->push(new AddEntityComponentCommand(m_globalInfo, entityIDs, "Collidable"));
}

void AddComponentMenu::addSoundListenerComponent() {
	std::set<EntityID> selectedEntityIDs = m_globalInfo.otherSelectedEntityIDs;
	selectedEntityIDs.insert(m_globalInfo.currentEntityID);

	std::vector<EntityID> entityIDs;
	for (EntityID selectedEntityID : selectedEntityIDs) {
		if (!m_globalInfo.entities[selectedEntityID].soundListener) {
			entityIDs.push_back(selectedEntityID);
		}
	}

	m_globalInfo.actionUndoStack->push(new AddEntityComponentCommand(m_globalInfo, entityIDs, "SoundListener"));
}

void AddComponentMenu::addScriptableComponent() {
	std::set<EntityID> selectedEntityIDs = m_globalInfo.otherSelectedEntityIDs;
	selectedEntityIDs.insert(m_globalInfo.currentEntityID);

	std::vector<EntityID> entityIDs;
	for (EntityID selectedEntityID : selectedEntityIDs) {
		if (!m_globalInfo.entities[selectedEntityID].scriptable) {
			entityIDs.push_back(selectedEntityID);
		}
	}

	m_globalInfo.actionUndoStack->push(new AddEntityComponentCommand(m_globalInfo, entityIDs, "Scriptable"));
}
