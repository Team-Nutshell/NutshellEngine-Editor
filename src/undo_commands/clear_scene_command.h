#pragma once
#include "../common/global_info.h"
#include <QUndoCommand>

class ClearSceneCommand : public QUndoCommand {
public:
	ClearSceneCommand(GlobalInfo& globalInfo, const std::string& previousScenePath);

	void undo();
	void redo();

private:
	GlobalInfo& m_globalInfo;

	std::unordered_map<EntityID, Entity> m_previousEntities;
	std::string m_previousScenePath;
};