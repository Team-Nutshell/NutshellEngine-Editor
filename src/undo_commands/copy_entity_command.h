#pragma once
#include "../common/global_info.h"
#include <QUndoCommand>
#include <string>

class CopyEntityCommand : public QUndoCommand {
public:
	CopyEntityCommand(GlobalInfo& globalInfo, Entity entity);

	void undo();
	void redo();

private:
	GlobalInfo& m_globalInfo;

	Entity m_copiedEntity;
	std::string m_passedEntityName;
	EntityID m_pastedEntityID;
};