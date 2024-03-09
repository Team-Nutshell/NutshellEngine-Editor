#pragma once
#include "../common/common.h"
#include "file_menu.h"
#include "edit_menu.h"
#include "view_menu.h"
#include "entity_panel.h"
#include "../renderer/renderer.h"
#include "entity_info_panel.h"
#include "resource_panel.h"
#include <QMainWindow>
#include <QWidget>
#include <QSplitter>
#include <memory>

class MainWindow : public QMainWindow {
	Q_OBJECT
public:
	MainWindow(GlobalInfo& globalInfo);

private:
	void createMenuBar();
	void createEntityPanel();
	void createRenderer();
	void createEntityInfoPanel();
	void createResourcePanel();

private:
	GlobalInfo& m_globalInfo;

	std::unique_ptr<QWidget> m_mainWidget;
	std::unique_ptr<QSplitter> m_horizontalSplitter;
	std::unique_ptr<QSplitter> m_verticalSplitter;
	std::unique_ptr<EntityPanel> m_entityPanel;
	std::unique_ptr<Renderer> m_renderer;
	std::unique_ptr<EntityInfoPanel> m_entityInfoPanel;
	std::unique_ptr<ResourcePanel> m_resourcePanel;
	std::unique_ptr<FileMenu> m_fileMenu;
	std::unique_ptr<EditMenu> m_editMenu;
	std::unique_ptr<ViewMenu> m_viewMenu;
};