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
#include "resource_panel.h"
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
	void createResourcePanel();
	void createLogBar();

private slots:
	void closeEvent(QCloseEvent* event);

private:
	GlobalInfo& m_globalInfo;

public:
	QWidget* mainWidget;
	QSplitter* horizontalSplitter;
	QSplitter* verticalSplitter;
	EntityPanel* entityPanel;
	Renderer* renderer;
	InfoPanel* infoPanel;
	ResourcePanel* resourcePanel;
	LogBar* logBar;
	BuildBar* buildBar;
	FileMenu* fileMenu;
	EditMenu* editMenu;
	ViewMenu* viewMenu;
	ProjectMenu* projectMenu;
	HelpMenu* helpMenu;
	OptionsMenu* optionsMenu;
};