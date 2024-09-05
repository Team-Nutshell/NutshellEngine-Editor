#pragma once
#include "../common/global_info.h"
#include <QUndoCommand>

class OpenSceneCommand : public QUndoCommand {
public:
	OpenSceneCommand(GlobalInfo& globalInfo, const std::unordered_map<EntityID, Entity>& previousEntities, const std::unordered_map<EntityID, Entity>& newEntities, const std::string& previousScenePath, const std::string& newScenePath, bool previousSceneModified);

	void undo();
	void redo();

private:
	GlobalInfo& m_globalInfo;

	std::unordered_map<EntityID, Entity> m_previousEntities;
	std::unordered_map<EntityID, Entity> m_newEntities;

	std::string m_previousScenePath;
	std::string m_newScenePath;

	bool m_previousSceneModified;
};