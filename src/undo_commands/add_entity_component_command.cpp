#include "add_entity_component_command.h"

AddEntityComponentCommand::AddEntityComponentCommand(GlobalInfo& globalInfo, EntityID entityID, const std::string& componentName) : m_globalInfo(globalInfo) {
	setText(QString::fromStdString(m_globalInfo.localization.getString("undo_add_component", { componentName, m_globalInfo.entities[entityID].name })));
	m_entityID = entityID;
	m_componentName = componentName;
}

void AddEntityComponentCommand::undo() {
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
	else if (m_componentName == "SoundListener") {
		m_globalInfo.entities[m_entityID].soundListener.reset();
		emit m_globalInfo.signalEmitter.removeEntitySoundListenerSignal(m_entityID);
	}
	else if (m_componentName == "Scriptable") {
		m_globalInfo.entities[m_entityID].scriptable.reset();
		emit m_globalInfo.signalEmitter.removeEntityScriptableSignal(m_entityID);
	}
}

void AddEntityComponentCommand::redo() {
	if (m_componentName == "Camera") {
		m_globalInfo.entities[m_entityID].camera = Camera();
		emit m_globalInfo.signalEmitter.addEntityCameraSignal(m_entityID);
	}
	else if (m_componentName == "Light") {
		m_globalInfo.entities[m_entityID].light = Light();
		emit m_globalInfo.signalEmitter.addEntityLightSignal(m_entityID);
	}
	else if (m_componentName == "Renderable") {
		m_globalInfo.entities[m_entityID].renderable = Renderable();
		emit m_globalInfo.signalEmitter.addEntityRenderableSignal(m_entityID);
	}
	else if (m_componentName == "Rigidbody") {
		m_globalInfo.entities[m_entityID].rigidbody = Rigidbody();
		emit m_globalInfo.signalEmitter.addEntityRigidbodySignal(m_entityID);
	}
	else if (m_componentName == "Collidable") {
		m_globalInfo.entities[m_entityID].collidable = Collidable();
		emit m_globalInfo.signalEmitter.addEntityCollidableSignal(m_entityID);
	}
	else if (m_componentName == "SoundListener") {
		m_globalInfo.entities[m_entityID].soundListener = SoundListener();
		emit m_globalInfo.signalEmitter.addEntitySoundListenerSignal(m_entityID);
	}
	else if (m_componentName == "Scriptable") {
		m_globalInfo.entities[m_entityID].scriptable = Scriptable();
		emit m_globalInfo.signalEmitter.addEntityScriptableSignal(m_entityID);
	}
}