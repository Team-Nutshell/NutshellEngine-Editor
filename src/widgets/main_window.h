#pragma once
#include "../common/common.h"
#include "file_menu.h"
#include "edit_menu.h"
#include "view_menu.h"
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

	QWidget* m_mainWidget;
	QSplitter* m_horizontalSplitter;
	QSplitter* m_verticalSplitter;
	EntityPanel* m_entityPanel;
	Renderer* m_renderer;
	EntityInfoPanel* m_entityInfoPanel;
	ResourcePanel* m_resourcePanel;
	LogBar* m_logBar;
	BuildBar* m_buildBar;
	FileMenu* m_fileMenu;
	EditMenu* m_editMenu;
	ViewMenu* m_viewMenu;
};