#pragma once
#include "../common/global_info.h"
#include <QUndoCommand>
#include <string>

class ChangeEntityComponentCommand : public QUndoCommand {
public:
	ChangeEntityComponentCommand(GlobalInfo& globalInfo, EntityID entityID, const std::string& componentName, Component* component);

	void undo();
	void redo();

private:
	GlobalInfo& m_globalInfo;

	EntityID m_entityID;
	std::string m_componentName;
	std::optional<Transform> m_previousTransform;
	std::optional<Transform> m_newTransform;
	std::optional<Camera> m_previousCamera;
	std::optional<Camera> m_newCamera;
	std::optional<Light> m_previousLight;
	std::optional<Light> m_newLight;
	std::optional<Renderable> m_previousRenderable;
	std::optional<Renderable> m_newRenderable;
	std::optional<Rigidbody> m_previousRigidbody;
	std::optional<Rigidbody> m_newRigidbody;
	std::optional<Collidable> m_previousCollidable;
	std::optional<Collidable> m_newCollidable;
	std::optional<Scriptable> m_previousScriptable;
	std::optional<Scriptable> m_newScriptable;
};