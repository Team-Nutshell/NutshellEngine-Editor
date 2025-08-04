#pragma once
#include "../common/global_info.h"
#include <QUndoCommand>

class ChangeEntitiesPersistenceCommand : public QUndoCommand {
public:
	ChangeEntitiesPersistenceCommand(GlobalInfo& globalInfo, const std::vector<EntityID>& entityIDs, bool isPersistent);

	void undo();
	void redo();

private:
	GlobalInfo& m_globalInfo;

	std::vector<EntityID> m_entityIDs;
	std::vector<bool> m_previousEntitiesPersistence;
	bool m_newEntitiesPersistence;
};