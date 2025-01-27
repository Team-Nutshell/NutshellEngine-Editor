#pragma once
#include "../common/global_info.h"
#include <QMenu>
#include <QAction>
#include <string>

class AssetListMenu : public QMenu {
	Q_OBJECT
public:
	AssetListMenu(GlobalInfo& globalInfo);

private slots:
	void renameAsset();
	void deleteAsset();
	void duplicateAsset();
	void reloadAsset();
	void newDirectory();
	void newModel();
	void newImageSampler();
	void newMaterial();
	void newScene();
	void copyPath();

private:
	GlobalInfo& m_globalInfo;

public:
	std::string directory = "";
	std::string filename = "";

	QAction* renameAction;
	QAction* deleteAction;
	QAction* duplicateAction;
	QAction* reloadAction;
	QAction* newDirectoryAction;
	QAction* newModelAction;
	QAction* newImageSamplerAction;
	QAction* newMaterialAction;
	QAction* newSceneAction;
	QAction* copyPathAction;
};