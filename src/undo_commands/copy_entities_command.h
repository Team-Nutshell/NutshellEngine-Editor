#pragma once
#include "../common/global_info.h"
#include <QUndoCommand>
#include <string>
#include <vector>

class CopyEntitiesCommand : public QUndoCommand {
public:
	CopyEntitiesCommand(GlobalInfo& globalInfo, std::vector<Entity> entities);

	void undo();
	void redo();

private:
	GlobalInfo& m_globalInfo;

	std::vector<Entity> m_copiedEntities;
	std::vector<std::string> m_pastedEntityNames;
	std::vector<EntityID> m_pastedEntityIDs;
};