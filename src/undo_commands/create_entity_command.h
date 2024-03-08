#pragma once
#include "../common/common.h"
#include <QUndoCommand>

class CreateEntityCommand : public QUndoCommand {
public:
	CreateEntityCommand(GlobalInfo& globalInfo, const std::string& name);

	void undo();
	void redo();

private:
	GlobalInfo& m_globalInfo;

	EntityID m_entityID;
	std::string m_entityName;
};