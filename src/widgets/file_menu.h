#pragma once
#include "../common/global_info.h"
#include <QMenu>
#include <QAction>

class FileMenu : public QMenu {
	Q_OBJECT
public:
	FileMenu(GlobalInfo& globalInfo);

private:
	void newScene();
	void openScene();
	void saveScene();
	void saveSceneAs();

private:
	GlobalInfo& m_globalInfo;

	QAction* m_newSceneAction;
	QAction* m_openSceneAction;
	QAction* m_saveSceneAction;
	QAction* m_saveSceneAsAction;
};