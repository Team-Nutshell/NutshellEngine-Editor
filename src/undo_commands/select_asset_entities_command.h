#pragma once
#include "../common/global_info.h"
#include <QUndoCommand>
#include <set>
#include <string>

class SelectAssetEntitiesCommand : public QUndoCommand {
public:
	SelectAssetEntitiesCommand(GlobalInfo& globalInfo, SelectionType newType, const std::string& newSelectedAssetPath, EntityID newCurrentEntityID, const std::set<EntityID>& newOtherSelectedEntityIDs);

	void undo();
	void redo();

private:
	GlobalInfo& m_globalInfo;

	SelectionType m_oldType;
	SelectionType m_newType;
	std::string m_oldSelectedAssetPath = "";
	std::string m_newSelectedAssetPath = "";
	EntityID m_oldCurrentEntityID = NO_ENTITY;
	EntityID m_newCurrentEntityID = NO_ENTITY;
	std::set<EntityID> m_oldOtherSelectedEntityIDs;
	std::set<EntityID> m_newOtherSelectedEntityIDs;
};