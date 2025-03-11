#pragma once
#include "../common/global_info.h"
#include <QUndoCommand>
#include <vector>
#include <string>

class ChangeEntitiesEntityGroupsCommand : public QUndoCommand {
public:
	ChangeEntitiesEntityGroupsCommand(GlobalInfo& globalInfo, const std::vector<EntityID>& entityIDs, const std::vector<std::string>& entityGroups);

	void undo();
	void redo();

private:
	GlobalInfo& m_globalInfo;

	std::vector<EntityID> m_entityIDs;
	std::vector<std::vector<std::string>> m_oldEntityGroups;
	std::vector<std::string> m_newEntityGroups;
};