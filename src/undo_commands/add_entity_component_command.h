#pragma once
#include "../common/global_info.h"
#include <QUndoCommand>
#include <string>

class AddEntityComponentCommand : public QUndoCommand {
public:
	AddEntityComponentCommand(GlobalInfo& globalInfo, EntityID entityID, const std::string& componentName);

	void undo();
	void redo();

private:
	GlobalInfo& m_globalInfo;

	EntityID m_entityID;
	std::string m_componentName;
};