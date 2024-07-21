#include "change_entity_component_command.h"

ChangeEntityComponentCommand::ChangeEntityComponentCommand(GlobalInfo& globalInfo, EntityID entityID, const std::string& componentName, Component* component) : m_globalInfo(globalInfo) {
	setText("Change " + QString::fromStdString(componentName) + " Component to Entity " + QString::fromStdString(m_globalInfo.entities[entityID].name));
	m_entityID = entityID;
	m_componentName = componentName;
	if (m_componentName == "Transform") {
		m_previousTransform = m_globalInfo.entities[m_entityID].transform;
		m_newTransform = *static_cast<Transform*>(component);
	}
	else if (m_componentName == "Camera") {
		m_previousCamera = m_globalInfo.entities[m_entityID].camera;
		m_newCamera = *static_cast<Camera*>(component);
	}
	else if (m_componentName == "Light") {
		m_previousLight = m_globalInfo.entities[m_entityID].light;
		m_newLight = *static_cast<Light*>(component);
	}
	else if (m_componentName == "Renderable") {
		m_previousRenderable = m_globalInfo.entities[m_entityID].renderable;
		m_newRenderable = *static_cast<Renderable*>(component);
	}
	else if (m_componentName == "Rigidbody") {
		m_previousRigidbody = m_globalInfo.entities[m_entityID].rigidbody;
		m_newRigidbody = *static_cast<Rigidbody*>(component);
	}
	else if (m_componentName == "Collidable") {
		m_previousCollidable = m_globalInfo.entities[m_entityID].collidable;
		m_newCollidable = *static_cast<Collidable*>(component);
	}
	else if (m_componentName == "Scriptable") {
		m_previousScriptable = m_globalInfo.entities[m_entityID].scriptable;
		m_newScriptable = *static_cast<Scriptable*>(component);
	}
}

void ChangeEntityComponentCommand::undo() {
	if (m_componentName == "Transform") {
		m_globalInfo.entities[m_entityID].transform = m_previousTransform.value();
		emit m_globalInfo.signalEmitter.changeEntityTransformSignal(m_entityID, m_previousTransform.value());
	}
	else if (m_componentName == "Camera") {
		m_globalInfo.entities[m_entityID].camera = m_previousCamera.value();
		emit m_globalInfo.signalEmitter.changeEntityCameraSignal(m_entityID, m_previousCamera.value());
	}
	else if (m_componentName == "Light") {
		m_globalInfo.entities[m_entityID].light = m_previousLight.value();
		emit m_globalInfo.signalEmitter.changeEntityLightSignal(m_entityID, m_previousLight.value());
	}
	else if (m_componentName == "Renderable") {
		m_globalInfo.entities[m_entityID].renderable = m_previousRenderable.value();
		emit m_globalInfo.signalEmitter.changeEntityRenderableSignal(m_entityID, m_previousRenderable.value());
	}
	else if (m_componentName == "Rigidbody") {
		m_globalInfo.entities[m_entityID].rigidbody = m_previousRigidbody.value();
		emit m_globalInfo.signalEmitter.changeEntityRigidbodySignal(m_entityID, m_previousRigidbody.value());
	}
	else if (m_componentName == "Collidable") {
		m_globalInfo.entities[m_entityID].collidable = m_previousCollidable.value();
		emit m_globalInfo.signalEmitter.changeEntityCollidableSignal(m_entityID, m_previousCollidable.value());
	}
	else if (m_componentName == "Scriptable") {
		m_globalInfo.entities[m_entityID].scriptable = m_previousScriptable.value();
		emit m_globalInfo.signalEmitter.changeEntityScriptableSignal(m_entityID, m_previousScriptable.value());
	}
}

void ChangeEntityComponentCommand::redo() {
	if (m_componentName == "Transform") {
		m_globalInfo.entities[m_entityID].transform = m_newTransform.value();
		emit m_globalInfo.signalEmitter.changeEntityTransformSignal(m_entityID, m_newTransform.value());
	}
	else if (m_componentName == "Camera") {
		m_globalInfo.entities[m_entityID].camera = m_newCamera.value();
		emit m_globalInfo.signalEmitter.changeEntityCameraSignal(m_entityID, m_newCamera.value());
	}
	else if (m_componentName == "Light") {
		m_globalInfo.entities[m_entityID].light = m_newLight.value();
		emit m_globalInfo.signalEmitter.changeEntityLightSignal(m_entityID, m_newLight.value());
	}
	else if (m_componentName == "Renderable") {
		m_globalInfo.entities[m_entityID].renderable = m_newRenderable.value();
		emit m_globalInfo.signalEmitter.changeEntityRenderableSignal(m_entityID, m_newRenderable.value());
	}
	else if (m_componentName == "Rigidbody") {
		m_globalInfo.entities[m_entityID].rigidbody = m_newRigidbody.value();
		emit m_globalInfo.signalEmitter.changeEntityRigidbodySignal(m_entityID, m_newRigidbody.value());
	}
	else if (m_componentName == "Collidable") {
		m_globalInfo.entities[m_entityID].collidable = m_newCollidable.value();
		emit m_globalInfo.signalEmitter.changeEntityCollidableSignal(m_entityID, m_newCollidable.value());
	}
	else if (m_componentName == "Scriptable") {
		m_globalInfo.entities[m_entityID].scriptable = m_newScriptable.value();
		emit m_globalInfo.signalEmitter.changeEntityScriptableSignal(m_entityID, m_newScriptable.value());
	}
}