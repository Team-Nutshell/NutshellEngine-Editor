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
	void rename();
	void newDirectory();
	void newModel();
	void newImageSampler();
	void newMaterial();
	void newScene();

	void onFileRenamed(const std::string& oldFilename, const std::string& newFilename);

private:
	GlobalInfo& m_globalInfo;

public:
	std::string directory = "";
	std::string filename = "";

	QAction* renameAction;
	QAction* newDirectoryAction;
	QAction* newModelAction;
	QAction* newImageSamplerAction;
	QAction* newMaterialAction;
	QAction* newSceneAction;
};