#include "add_entity_component_command.h"

AddEntityComponentCommand::AddEntityComponentCommand(GlobalInfo& globalInfo, const std::vector<EntityID>& entityIDs, const std::string& componentName) : m_globalInfo(globalInfo) {
	if (entityIDs.size() == 1) {
		setText(QString::fromStdString(m_globalInfo.localization.getString("undo_add_component_entity", { componentName, m_globalInfo.entities[entityIDs[0]].name })));
	}
	else {
		setText(QString::fromStdString(m_globalInfo.localization.getString("undo_add_component_entities", { componentName })));
	}
	m_entityIDs = entityIDs;
	m_componentName = componentName;
}

void AddEntityComponentCommand::undo() {
	for (size_t i = 0; i < m_entityIDs.size(); i++) {
		if (m_componentName == "Camera") {
			m_globalInfo.entities[m_entityIDs[i]].camera.reset();
			emit m_globalInfo.signalEmitter.removeEntityCameraSignal(m_entityIDs[i]);
		}
		else if (m_componentName == "Light") {
			m_globalInfo.entities[m_entityIDs[i]].light.reset();
			emit m_globalInfo.signalEmitter.removeEntityLightSignal(m_entityIDs[i]);
		}
		else if (m_componentName == "Renderable") {
			m_globalInfo.entities[m_entityIDs[i]].renderable.reset();
			emit m_globalInfo.signalEmitter.removeEntityRenderableSignal(m_entityIDs[i]);
		}
		else if (m_componentName == "Rigidbody") {
			m_globalInfo.entities[m_entityIDs[i]].rigidbody.reset();
			emit m_globalInfo.signalEmitter.removeEntityRigidbodySignal(m_entityIDs[i]);
		}
		else if (m_componentName == "Collidable") {
			m_globalInfo.entities[m_entityIDs[i]].collidable.reset();
			emit m_globalInfo.signalEmitter.removeEntityCollidableSignal(m_entityIDs[i]);
		}
		else if (m_componentName == "SoundListener") {
			m_globalInfo.entities[m_entityIDs[i]].soundListener.reset();
			emit m_globalInfo.signalEmitter.removeEntitySoundListenerSignal(m_entityIDs[i]);
		}
		else if (m_componentName == "Scriptable") {
			m_globalInfo.entities[m_entityIDs[i]].scriptable.reset();
			emit m_globalInfo.signalEmitter.removeEntityScriptableSignal(m_entityIDs[i]);
		}
	}
}

void AddEntityComponentCommand::redo() {
	for (size_t i = 0; i < m_entityIDs.size(); i++) {
		if (m_componentName == "Camera") {
			m_globalInfo.entities[m_entityIDs[i]].camera = Camera();
			emit m_globalInfo.signalEmitter.addEntityCameraSignal(m_entityIDs[i]);
		}
		else if (m_componentName == "Light") {
			m_globalInfo.entities[m_entityIDs[i]].light = Light();
			emit m_globalInfo.signalEmitter.addEntityLightSignal(m_entityIDs[i]);
		}
		else if (m_componentName == "Renderable") {
			m_globalInfo.entities[m_entityIDs[i]].renderable = Renderable();
			emit m_globalInfo.signalEmitter.addEntityRenderableSignal(m_entityIDs[i]);
		}
		else if (m_componentName == "Rigidbody") {
			m_globalInfo.entities[m_entityIDs[i]].rigidbody = Rigidbody();
			emit m_globalInfo.signalEmitter.addEntityRigidbodySignal(m_entityIDs[i]);
		}
		else if (m_componentName == "Collidable") {
			m_globalInfo.entities[m_entityIDs[i]].collidable = Collidable();
			emit m_globalInfo.signalEmitter.addEntityCollidableSignal(m_entityIDs[i]);
		}
		else if (m_componentName == "SoundListener") {
			m_globalInfo.entities[m_entityIDs[i]].soundListener = SoundListener();
			emit m_globalInfo.signalEmitter.addEntitySoundListenerSignal(m_entityIDs[i]);
		}
		else if (m_componentName == "Scriptable") {
			m_globalInfo.entities[m_entityIDs[i]].scriptable = Scriptable();
			emit m_globalInfo.signalEmitter.addEntityScriptableSignal(m_entityIDs[i]);
		}
	}
}