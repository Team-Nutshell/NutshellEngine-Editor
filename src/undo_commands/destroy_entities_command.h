#pragma once
#include "../common/global_info.h"
#include <QUndoCommand>
#include <utility>

class DestroyEntitiesCommand : public QUndoCommand {
public:
	DestroyEntitiesCommand(GlobalInfo& globalInfo, const std::vector<EntityID>& entityIDs);

	void undo();
	void redo();

private:
	GlobalInfo& m_globalInfo;

	std::vector<std::pair<Entity, int>> m_destroyedEntities;
};