#pragma once
#include "../common/global_info.h"
#include <QUndoCommand>

class OpenSceneCommand : public QUndoCommand {
public:
	OpenSceneCommand(GlobalInfo& globalInfo, const std::unordered_map<EntityID, Entity>& entities, const std::string& scenePath);

	void undo();
	void redo();

private:
	GlobalInfo& m_globalInfo;

	std::unordered_map<EntityID, Entity> m_newEntities;
};