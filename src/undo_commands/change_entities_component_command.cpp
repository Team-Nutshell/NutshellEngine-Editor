#include "change_entities_component_command.h"

ChangeEntitiesComponentCommand::ChangeEntitiesComponentCommand(GlobalInfo& globalInfo, const std::vector<EntityID>& entityIDs, const std::string& componentName, const std::vector<Component*>& components) : m_globalInfo(globalInfo) {
	if (entityIDs.size() == 1) {
		setText(QString::fromStdString(m_globalInfo.localization.getString("undo_change_component_entity", { componentName, m_globalInfo.entities[entityIDs[0]].name })));
	}
	else {
		setText(QString::fromStdString(m_globalInfo.localization.getString("undo_change_component_entities", { componentName })));
	}
	m_entityIDs = entityIDs;
	m_componentName = componentName;
	for (size_t i = 0; i < m_entityIDs.size(); i++) {
		if (m_componentName == "Transform") {
			m_previousTransforms.push_back(m_globalInfo.entities[m_entityIDs[i]].transform);
			m_newTransforms.push_back(*static_cast<Transform*>(components[i]));
		}
		else if (m_componentName == "Camera") {
			m_previousCameras.push_back(m_globalInfo.entities[m_entityIDs[i]].camera.value());
			m_newCameras.push_back(*static_cast<Camera*>(components[i]));
		}
		else if (m_componentName == "Light") {
			m_previousLights.push_back(m_globalInfo.entities[m_entityIDs[i]].light.value());
			m_newLights.push_back(*static_cast<Light*>(components[i]));
		}
		else if (m_componentName == "Renderable") {
			m_previousRenderables.push_back(m_globalInfo.entities[m_entityIDs[i]].renderable.value());
			m_newRenderables.push_back(*static_cast<Renderable*>(components[i]));
		}
		else if (m_componentName == "Rigidbody") {
			m_previousRigidbodys.push_back(m_globalInfo.entities[m_entityIDs[i]].rigidbody.value());
			m_newRigidbodys.push_back(*static_cast<Rigidbody*>(components[i]));
		}
		else if (m_componentName == "Collidable") {
			m_previousCollidables.push_back(m_globalInfo.entities[m_entityIDs[i]].collidable.value());
			m_newCollidables.push_back(*static_cast<Collidable*>(components[i]));
		}
		else if (m_componentName == "SoundListener") {
			m_previousSoundListeners.push_back(m_globalInfo.entities[m_entityIDs[i]].soundListener.value());
			m_newSoundListeners.push_back(*static_cast<SoundListener*>(components[i]));
		}
		else if (m_componentName == "Scriptable") {
			m_previousScriptables.push_back(m_globalInfo.entities[m_entityIDs[i]].scriptable.value());
			m_newScriptables.push_back(*static_cast<Scriptable*>(components[i]));
		}
	}
}

void ChangeEntitiesComponentCommand::undo() {
	for (size_t i = 0; i < m_entityIDs.size(); i++) {
		if (m_componentName == "Transform") {
			m_globalInfo.entities[m_entityIDs[i]].transform = m_previousTransforms[i];
			emit m_globalInfo.signalEmitter.changeEntityTransformSignal(m_entityIDs[i], m_globalInfo.entities[m_entityIDs[i]].transform);
		}
		else if (m_componentName == "Camera") {
			m_globalInfo.entities[m_entityIDs[i]].camera = m_previousCameras[i];
			emit m_globalInfo.signalEmitter.changeEntityCameraSignal(m_entityIDs[i], m_globalInfo.entities[m_entityIDs[i]].camera.value());
		}
		else if (m_componentName == "Light") {
			m_globalInfo.entities[m_entityIDs[i]].light = m_previousLights[i];
			emit m_globalInfo.signalEmitter.changeEntityLightSignal(m_entityIDs[i], m_globalInfo.entities[m_entityIDs[i]].light.value());
		}
		else if (m_componentName == "Renderable") {
			m_globalInfo.entities[m_entityIDs[i]].renderable = m_previousRenderables[i];
			emit m_globalInfo.signalEmitter.changeEntityRenderableSignal(m_entityIDs[i], m_globalInfo.entities[m_entityIDs[i]].renderable.value());
		}
		else if (m_componentName == "Rigidbody") {
			m_globalInfo.entities[m_entityIDs[i]].rigidbody = m_previousRigidbodys[i];
			emit m_globalInfo.signalEmitter.changeEntityRigidbodySignal(m_entityIDs[i], m_globalInfo.entities[m_entityIDs[i]].rigidbody.value());
		}
		else if (m_componentName == "Collidable") {
			m_globalInfo.entities[m_entityIDs[i]].collidable = m_previousCollidables[i];
			emit m_globalInfo.signalEmitter.changeEntityCollidableSignal(m_entityIDs[i], m_globalInfo.entities[m_entityIDs[i]].collidable.value());
		}
		else if (m_componentName == "SoundListener") {
			m_globalInfo.entities[m_entityIDs[i]].soundListener = m_previousSoundListeners[i];
			emit m_globalInfo.signalEmitter.changeEntitySoundListenerSignal(m_entityIDs[i], m_globalInfo.entities[m_entityIDs[i]].soundListener.value());
		}
		else if (m_componentName == "Scriptable") {
			m_globalInfo.entities[m_entityIDs[i]].scriptable = m_previousScriptables[i];
			emit m_globalInfo.signalEmitter.changeEntityScriptableSignal(m_entityIDs[i], m_globalInfo.entities[m_entityIDs[i]].scriptable.value());
		}
	}
}

void ChangeEntitiesComponentCommand::redo() {
	for (size_t i = 0; i < m_entityIDs.size(); i++) {
		if (m_componentName == "Transform") {
			m_globalInfo.entities[m_entityIDs[i]].transform = m_newTransforms[i];
			emit m_globalInfo.signalEmitter.changeEntityTransformSignal(m_entityIDs[i], m_globalInfo.entities[m_entityIDs[i]].transform);
		}
		else if (m_componentName == "Camera") {
			m_globalInfo.entities[m_entityIDs[i]].camera = m_newCameras[i];
			emit m_globalInfo.signalEmitter.changeEntityCameraSignal(m_entityIDs[i], m_globalInfo.entities[m_entityIDs[i]].camera.value());
		}
		else if (m_componentName == "Light") {
			m_globalInfo.entities[m_entityIDs[i]].light = m_newLights[i];
			emit m_globalInfo.signalEmitter.changeEntityLightSignal(m_entityIDs[i], m_globalInfo.entities[m_entityIDs[i]].light.value());
		}
		else if (m_componentName == "Renderable") {
			m_globalInfo.entities[m_entityIDs[i]].renderable = m_newRenderables[i];
			emit m_globalInfo.signalEmitter.changeEntityRenderableSignal(m_entityIDs[i], m_globalInfo.entities[m_entityIDs[i]].renderable.value());
		}
		else if (m_componentName == "Rigidbody") {
			m_globalInfo.entities[m_entityIDs[i]].rigidbody = m_newRigidbodys[i];
			emit m_globalInfo.signalEmitter.changeEntityRigidbodySignal(m_entityIDs[i], m_globalInfo.entities[m_entityIDs[i]].rigidbody.value());
		}
		else if (m_componentName == "Collidable") {
			m_globalInfo.entities[m_entityIDs[i]].collidable = m_newCollidables[i];
			emit m_globalInfo.signalEmitter.changeEntityCollidableSignal(m_entityIDs[i], m_globalInfo.entities[m_entityIDs[i]].collidable.value());
		}
		else if (m_componentName == "SoundListener") {
			m_globalInfo.entities[m_entityIDs[i]].soundListener = m_newSoundListeners[i];
			emit m_globalInfo.signalEmitter.changeEntitySoundListenerSignal(m_entityIDs[i], m_globalInfo.entities[m_entityIDs[i]].soundListener.value());
		}
		else if (m_componentName == "Scriptable") {
			m_globalInfo.entities[m_entityIDs[i]].scriptable = m_newScriptables[i];
			emit m_globalInfo.signalEmitter.changeEntityScriptableSignal(m_entityIDs[i], m_globalInfo.entities[m_entityIDs[i]].scriptable.value());
		}
	}
}