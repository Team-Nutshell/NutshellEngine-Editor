#pragma once
#include "../common/global_info.h"
#include <QUndoCommand>
#include <string>
#include <vector>

class AddEntitiesComponentCommand : public QUndoCommand {
public:
	AddEntitiesComponentCommand(GlobalInfo& globalInfo, const std::vector<EntityID>& entityIDs, const std::string& componentName);

	void undo();
	void redo();

private:
	GlobalInfo& m_globalInfo;

	std::vector<EntityID> m_entityIDs;
	std::string m_componentName;
};