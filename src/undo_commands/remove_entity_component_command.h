#pragma once
#include "../common/global_info.h"
#include <QUndoCommand>
#include <string>

class RemoveEntityComponentCommand : public QUndoCommand {
public:
	RemoveEntityComponentCommand(GlobalInfo& globalInfo, EntityID entityID, const std::string& componentName);

	void undo();
	void redo();

private:
	GlobalInfo& m_globalInfo;

	EntityID m_entityID;
	std::string m_componentName;
	std::optional<Camera> m_camera;
	std::optional<Light> m_light;
	std::optional<Renderable> m_renderable;
	std::optional<Rigidbody> m_rigidbody;
	std::optional<Collidable> m_collidable;
	std::optional<Scriptable> m_scriptable;
};