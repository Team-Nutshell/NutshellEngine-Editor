#include "main_window.h"
#include "separator_line.h"
#include <QVBoxLayout>
#include <QMenuBar>

MainWindow::MainWindow(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	resize(1280, 720);
	setWindowTitle("NutshellEngine - " + QString::fromStdString(globalInfo.projectName));
	setWindowIcon(QIcon("assets/icon.png"));
	setAttribute(Qt::WA_DeleteOnClose);

	m_mainWidget = new QWidget();
	m_horizontalSplitter = new QSplitter();
	m_horizontalSplitter->setOrientation(Qt::Orientation::Horizontal);
	setCentralWidget(m_mainWidget);
	createMenuBar();
	createEntityPanel();
	createRenderer();
	createEntityInfoPanel();
	m_verticalSplitter = new QSplitter();
	m_verticalSplitter->setOrientation(Qt::Orientation::Vertical);
	m_verticalSplitter->addWidget(m_horizontalSplitter);
	m_mainWidget->setLayout(new QVBoxLayout());
	m_mainWidget->layout()->setContentsMargins(0, 0, 0, 0);
	m_mainWidget->layout()->addWidget(m_verticalSplitter);
	createResourcePanel();
	m_verticalSplitter->setSizes({ 520, 200 });
	createLogBar();
}

void MainWindow::createMenuBar() {
	m_fileMenu = new FileMenu(m_globalInfo);
	menuBar()->addMenu(m_fileMenu);
	m_editMenu = new EditMenu(m_globalInfo);
	menuBar()->addMenu(m_editMenu);
	m_viewMenu = new ViewMenu(m_globalInfo);
	menuBar()->addMenu(m_viewMenu);
}

void MainWindow::createEntityPanel() {
	m_entityPanel = new EntityPanel(m_globalInfo);
	m_horizontalSplitter->addWidget(m_entityPanel);
}

void MainWindow::createRenderer() {
	m_renderer = new Renderer(m_globalInfo);
	m_horizontalSplitter->addWidget(m_renderer);
}

void MainWindow::createEntityInfoPanel() {
	m_entityInfoPanel = new EntityInfoPanel(m_globalInfo);
	m_horizontalSplitter->addWidget(m_entityInfoPanel);
}

void MainWindow::createResourcePanel() {
	m_resourcePanel = new ResourcePanel(m_globalInfo);
	m_verticalSplitter->addWidget(m_resourcePanel);
}

void MainWindow::createLogBar() {
	m_logBar = new LogBar(m_globalInfo);
	m_mainWidget->layout()->addWidget(m_logBar);
	m_globalInfo.logger.logBar = m_logBar;
}
