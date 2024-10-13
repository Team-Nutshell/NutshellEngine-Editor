#pragma once
#include "../common/global_info.h"
#include <QUndoCommand>

class OpenSceneCommand : public QUndoCommand {
public:
	OpenSceneCommand(GlobalInfo& globalInfo, const std::vector<Entity>& newEntities, const std::string& newScenePath);

	void undo();
	void redo();

private:
	GlobalInfo& m_globalInfo;

	std::vector<Entity> m_previousEntities;
	std::vector<Entity> m_newEntities;

	std::string m_previousScenePath;
	std::string m_newScenePath;

	bool m_previousSceneModified;
};