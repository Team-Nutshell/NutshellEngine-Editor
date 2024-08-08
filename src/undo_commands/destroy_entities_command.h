#pragma once
#include "../common/global_info.h"
#include <QUndoCommand>

class DestroyEntitiesCommand : public QUndoCommand {
public:
	DestroyEntitiesCommand(GlobalInfo& globalInfo, const std::vector<EntityID>& entityIDs);

	void undo();
	void redo();

private:
	GlobalInfo& m_globalInfo;

	std::vector<Entity> m_destroyedEntities;
};