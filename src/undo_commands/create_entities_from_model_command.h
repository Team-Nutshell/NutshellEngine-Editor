#pragma once
#include "../common/global_info.h"
#include <QUndoCommand>

class CreateEntitiesFromModelCommand : public QUndoCommand {
public:
	CreateEntitiesFromModelCommand(GlobalInfo& globalInfo, const std::string& name, const std::string& modelPath);

	void undo();
	void redo();

private:
	GlobalInfo& m_globalInfo;

	std::vector<EntityID> m_entityIDs;
	std::string m_baseEntityName = "";
	std::string m_modelPath = "";
};