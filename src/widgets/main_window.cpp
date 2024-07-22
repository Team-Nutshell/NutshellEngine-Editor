#include "main_window.h"
#include "separator_line.h"
#include <QVBoxLayout>
#include <QMenuBar>

MainWindow::MainWindow(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	resize(1280, 720);
	setWindowTitle("NutshellEngine - " + QString::fromStdString(globalInfo.projectName));
	setWindowIcon(QIcon("assets/icon.png"));
	setAttribute(Qt::WA_DeleteOnClose);

	mainWidget = new QWidget();
	mainWidget->setLayout(new QVBoxLayout());
	mainWidget->layout()->setContentsMargins(0, 0, 0, 0);
	horizontalSplitter = new QSplitter();
	horizontalSplitter->setOrientation(Qt::Orientation::Horizontal);
	setCentralWidget(mainWidget);
	createMenuBar();
	createBuildBar();
	createEntityPanel();
	createRenderer();
	createEntityInfoPanel();
	verticalSplitter = new QSplitter();
	verticalSplitter->setOrientation(Qt::Orientation::Vertical);
	verticalSplitter->addWidget(horizontalSplitter);
	mainWidget->layout()->addWidget(verticalSplitter);
	createResourcePanel();
	verticalSplitter->setSizes({ 520, 200 });
	createLogBar();
}

void MainWindow::createMenuBar() {
	fileMenu = new FileMenu(m_globalInfo);
	menuBar()->addMenu(fileMenu);
	editMenu = new EditMenu(m_globalInfo);
	menuBar()->addMenu(editMenu);
	viewMenu = new ViewMenu(m_globalInfo);
	menuBar()->addMenu(viewMenu);
	optionsMenu = new OptionsMenu(m_globalInfo);
	menuBar()->addMenu(optionsMenu);
}

void MainWindow::createBuildBar() {
	buildBar = new BuildBar(m_globalInfo);
	mainWidget->layout()->addWidget(buildBar);
}

void MainWindow::createEntityPanel() {
	entityPanel = new EntityPanel(m_globalInfo);
	horizontalSplitter->addWidget(entityPanel);
}

void MainWindow::createRenderer() {
	renderer = new Renderer(m_globalInfo);
	horizontalSplitter->addWidget(renderer);
}

void MainWindow::createEntityInfoPanel() {
	entityInfoPanel = new EntityInfoPanel(m_globalInfo);
	horizontalSplitter->addWidget(entityInfoPanel);
}

void MainWindow::createResourcePanel() {
	resourcePanel = new ResourcePanel(m_globalInfo);
	verticalSplitter->addWidget(resourcePanel);
}

void MainWindow::createLogBar() {
	logBar = new LogBar(m_globalInfo);
	mainWidget->layout()->addWidget(logBar);
}
