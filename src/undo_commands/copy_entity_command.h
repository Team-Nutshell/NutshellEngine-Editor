#pragma once
#include "../common/common.h"
#include <QUndoCommand>
#include <string>

class CopyEntityCommand : public QUndoCommand {
public:
	CopyEntityCommand(GlobalInfo& globalInfo, EntityID entityID);

	void undo();
	void redo();

private:
	GlobalInfo& m_globalInfo;

	Entity m_copiedEntity;
	std::string m_passedEntityName;
	EntityID m_pastedEntityID;
};