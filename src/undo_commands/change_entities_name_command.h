#pragma once
#include "../common/global_info.h"
#include <QUndoCommand>
#include <string>

class ChangeEntitiesNameCommand : public QUndoCommand {
public:
	ChangeEntitiesNameCommand(GlobalInfo& globalInfo, std::vector<EntityID> entityIDs, const std::string& name);

	void undo();
	void redo();

private:
	GlobalInfo& m_globalInfo;

	std::vector<EntityID> m_entityIDs;
	std::vector<std::string> m_previousEntityNames;
	std::vector<std::string> m_newEntityNames;
};