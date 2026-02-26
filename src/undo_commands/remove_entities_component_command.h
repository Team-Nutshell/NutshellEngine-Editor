#pragma once
#include "../common/global_info.h"
#include <QUndoCommand>
#include <string>
#include <unordered_map>

class RemoveEntitiesComponentCommand : public QUndoCommand {
public:
	RemoveEntitiesComponentCommand(GlobalInfo& globalInfo, const std::vector<EntityID>& entityIDs, const std::string& componentName);

	void undo();
	void redo();

private:
	GlobalInfo& m_globalInfo;

	std::vector<EntityID> m_entityIDs;
	std::string m_componentName;
	std::unordered_map<EntityID, std::optional<Camera>> m_cameras;
	std::unordered_map<EntityID, std::optional<Light>> m_lights;
	std::unordered_map<EntityID, std::optional<Renderable>> m_renderables;
	std::unordered_map<EntityID, std::optional<Rigidbody>> m_rigidbodies;
	std::unordered_map<EntityID, std::optional<Collidable>> m_collidables;
	std::unordered_map<EntityID, std::optional<SoundListener>> m_soundListeners;
	std::unordered_map<EntityID, std::optional<Scriptable>> m_scriptables;
};