#include "remove_entity_component_command.h"

RemoveEntityComponentCommand::RemoveEntityComponentCommand(GlobalInfo& globalInfo, EntityID entityID, const std::string& componentName) : m_globalInfo(globalInfo) {
	setText(QString::fromStdString(m_globalInfo.localization.getString("undo_remove_component", { componentName, m_globalInfo.entities[entityID].name })));
	m_entityID = entityID;
	m_componentName = componentName;
	if (m_componentName == "Camera") {
		m_camera = m_globalInfo.entities[m_entityID].camera;
	}
	else if (m_componentName == "Light") {
		m_light = m_globalInfo.entities[m_entityID].light;
	}
	else if (m_componentName == "Renderable") {
		m_renderable = m_globalInfo.entities[m_entityID].renderable;
	}
	else if (m_componentName == "Rigidbody") {
		m_rigidbody = m_globalInfo.entities[m_entityID].rigidbody;
	}
	else if (m_componentName == "Collidable") {
		m_collidable = m_globalInfo.entities[m_entityID].collidable;
	}
	else if (m_componentName == "Scriptable") {
		m_scriptable = m_globalInfo.entities[m_entityID].scriptable;
	}
}

void RemoveEntityComponentCommand::undo() {
	if (m_componentName == "Camera") {
		m_globalInfo.entities[m_entityID].camera = m_camera;
		emit m_globalInfo.signalEmitter.addEntityCameraSignal(m_entityID);
	}
	else if (m_componentName == "Light") {
		m_globalInfo.entities[m_entityID].light = m_light;
		emit m_globalInfo.signalEmitter.addEntityLightSignal(m_entityID);
	}
	else if (m_componentName == "Renderable") {
		m_globalInfo.entities[m_entityID].renderable = m_renderable;
		emit m_globalInfo.signalEmitter.addEntityRenderableSignal(m_entityID);
	}
	else if (m_componentName == "Rigidbody") {
		m_globalInfo.entities[m_entityID].rigidbody = m_rigidbody;
		emit m_globalInfo.signalEmitter.addEntityRigidbodySignal(m_entityID);
	}
	else if (m_componentName == "Collidable") {
		m_globalInfo.entities[m_entityID].collidable = m_collidable;
		emit m_globalInfo.signalEmitter.addEntityCollidableSignal(m_entityID);
	}
	else if (m_componentName == "Scriptable") {
		m_globalInfo.entities[m_entityID].scriptable = m_scriptable;
		emit m_globalInfo.signalEmitter.addEntityScriptableSignal(m_entityID);
	}
}

void RemoveEntityComponentCommand::redo() {
	if (m_componentName == "Camera") {
		m_globalInfo.entities[m_entityID].camera.reset();
		emit m_globalInfo.signalEmitter.removeEntityCameraSignal(m_entityID);
	}
	else if (m_componentName == "Light") {
		m_globalInfo.entities[m_entityID].light.reset();
		emit m_globalInfo.signalEmitter.removeEntityLightSignal(m_entityID);
	}
	else if (m_componentName == "Renderable") {
		m_globalInfo.entities[m_entityID].renderable.reset();
		emit m_globalInfo.signalEmitter.removeEntityRenderableSignal(m_entityID);
	}
	else if (m_componentName == "Rigidbody") {
		m_globalInfo.entities[m_entityID].rigidbody.reset();
		emit m_globalInfo.signalEmitter.removeEntityRigidbodySignal(m_entityID);
	}
	else if (m_componentName == "Collidable") {
		m_globalInfo.entities[m_entityID].collidable.reset();
		emit m_globalInfo.signalEmitter.removeEntityCollidableSignal(m_entityID);
	}
	else if (m_componentName == "Scriptable") {
		m_globalInfo.entities[m_entityID].scriptable.reset();
		emit m_globalInfo.signalEmitter.removeEntityScriptableSignal(m_entityID);
	}
}