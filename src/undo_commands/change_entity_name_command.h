#pragma once
#include "../common/global_info.h"
#include <QUndoCommand>
#include <string>

class ChangeEntityNameCommand : public QUndoCommand {
public:
	ChangeEntityNameCommand(GlobalInfo& globalInfo, EntityID entityID, const std::string& name);

	void undo();
	void redo();

private:
	GlobalInfo& m_globalInfo;

	EntityID m_entityID;
	std::string m_previousEntityName;
	std::string m_newEntityName;
};