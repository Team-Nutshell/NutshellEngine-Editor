#pragma once
#include "../common/global_info.h"
#include "file_menu.h"
#include "edit_menu.h"
#include "view_menu.h"
#include "project_menu.h"
#include "options_menu.h"
#include "help_menu.h"
#include "build_bar.h"
#include "entity_panel.h"
#include "log_bar.h"
#include "info_panel.h"
#include "resource_splitter.h"
#include "../renderer/renderer.h"
#include <QMainWindow>
#include <QVBoxLayout>
#include <QWidget>
#include <QSplitter>
#include <QCloseEvent>

class MainWindow : public QMainWindow {
	Q_OBJECT
public:
	MainWindow(GlobalInfo& globalInfo);

	void updateTitle();

private:
	void createMenuBar();
	void createBuildBar();
	void createEntityPanel();
	void createRenderer();
	void createInfoPanel();
	void createResourceSplitter();
	void createLogBar();

private slots:
	void closeEvent(QCloseEvent* event);

	void onCloseSceneConfirmed();

private:
	GlobalInfo& m_globalInfo;

	bool m_forceClose = false;

public:
	QWidget* mainWidget;
	QSplitter* horizontalSplitter;
	QSplitter* verticalSplitter;
	EntityPanel* entityPanel;
	Renderer* renderer;
	InfoPanel* infoPanel;
	ResourceSplitter* resourceSplitter;
	LogBar* logBar;
	BuildBar* buildBar;
	FileMenu* fileMenu;
	EditMenu* editMenu;
	ViewMenu* viewMenu;
	ProjectMenu* projectMenu;
	HelpMenu* helpMenu;
	OptionsMenu* optionsMenu;
};