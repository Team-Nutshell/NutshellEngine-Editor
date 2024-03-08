#pragma once
#include "../common/common.h"
#include <QUndoCommand>

class DestroyEntityCommand : public QUndoCommand {
public:
	DestroyEntityCommand(GlobalInfo& globalInfo, EntityID entityID);

	void undo();
	void redo();

private:
	GlobalInfo& m_globalInfo;

	Entity m_destroyedEntity;
};