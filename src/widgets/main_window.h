#pragma once
#include "../common/global_info.h"
#include "file_menu.h"
#include "edit_menu.h"
#include "view_menu.h"
#include "options_menu.h"
#include "build_bar.h"
#include "entity_panel.h"
#include "log_bar.h"
#include "../renderer/renderer.h"
#include "entity_info_panel.h"
#include "resource_panel.h"
#include <QMainWindow>
#include <QVBoxLayout>
#include <QWidget>
#include <QSplitter>

class MainWindow : public QMainWindow {
	Q_OBJECT
public:
	MainWindow(GlobalInfo& globalInfo);

private:
	void createMenuBar();
	void createBuildBar();
	void createEntityPanel();
	void createRenderer();
	void createEntityInfoPanel();
	void createResourcePanel();
	void createLogBar();

private:
	GlobalInfo& m_globalInfo;

public:
	QWidget* mainWidget;
	QSplitter* horizontalSplitter;
	QSplitter* verticalSplitter;
	EntityPanel* entityPanel;
	Renderer* renderer;
	EntityInfoPanel* entityInfoPanel;
	ResourcePanel* resourcePanel;
	LogBar* logBar;
	BuildBar* buildBar;
	FileMenu* fileMenu;
	EditMenu* editMenu;
	ViewMenu* viewMenu;
	OptionsMenu* optionsMenu;
};