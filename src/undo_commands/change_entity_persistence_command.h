#pragma once
#include "../common/common.h"
#include <QUndoCommand>

class ChangeEntityPersistenceCommand : public QUndoCommand {
public:
	ChangeEntityPersistenceCommand(GlobalInfo& globalInfo, EntityID entityID, bool isPersistent);

	void undo();
	void redo();

private:
	GlobalInfo& m_globalInfo;

	EntityID m_entityID;
	bool m_previousEntityPersistence;
	bool m_newEntityPersistence;
};