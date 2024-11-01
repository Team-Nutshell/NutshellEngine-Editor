#pragma once
#include "../common/global_info.h"
#include <QUndoCommand>
#include <string>

class ChangeEntitiesComponentCommand : public QUndoCommand {
public:
	ChangeEntitiesComponentCommand(GlobalInfo& globalInfo, const std::vector<EntityID>& entityIDs, const std::string& componentName, const std::vector<Component*>& components);

	void undo();
	void redo();

private:
	GlobalInfo& m_globalInfo;

	std::vector<EntityID> m_entityIDs;
	std::string m_componentName;
	std::vector<Transform> m_previousTransforms;
	std::vector<Transform> m_newTransforms;
	std::vector<Camera> m_previousCameras;
	std::vector<Camera> m_newCameras;
	std::vector<Light> m_previousLights;
	std::vector<Light> m_newLights;
	std::vector<Renderable> m_previousRenderables;
	std::vector<Renderable> m_newRenderables;
	std::vector<Rigidbody> m_previousRigidbodies;
	std::vector<Rigidbody> m_newRigidbodies;
	std::vector<Collidable> m_previousCollidables;
	std::vector<Collidable> m_newCollidables;
	std::vector<SoundListener> m_previousSoundListeners;
	std::vector<SoundListener> m_newSoundListeners;
	std::vector<Scriptable> m_previousScriptables;
	std::vector<Scriptable> m_newScriptables;
};