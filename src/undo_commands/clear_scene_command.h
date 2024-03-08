#pragma once
#include "../common/common.h"
#include <QUndoCommand>

class ClearSceneCommand : public QUndoCommand {
public:
	ClearSceneCommand(GlobalInfo& globalInfo);

	void undo();
	void redo();

private:
	GlobalInfo& m_globalInfo;

	std::unordered_map<EntityID, Entity> m_previousEntities;
};