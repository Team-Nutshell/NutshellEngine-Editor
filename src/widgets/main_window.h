#pragma once
#include "../common/common.h"
#include "file_menu.h"
#include "edit_menu.h"
#include "view_menu.h"
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
	void createRendererPanel();
	void createEntityInfoPanel();
	void createResourcePanel();

private:
	GlobalInfo& m_globalInfo;

	std::unique_ptr<QWidget> m_mainWidget;
	std::unique_ptr<QSplitter> m_horizontalSplitter;
	std::unique_ptr<QSplitter> m_verticalSplitter;
	std::unique_ptr<FileMenu> m_fileMenu;
	std::unique_ptr<EditMenu> m_editMenu;
	std::unique_ptr<ViewMenu> m_viewMenu;
};