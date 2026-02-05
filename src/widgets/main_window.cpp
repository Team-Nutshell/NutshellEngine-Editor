#include "main_window.h"
#include "separator_line.h"
#include "close_scene_widget.h"
#include "../common/asset_helper.h"
#include <QVBoxLayout>
#include <QMenuBar>
#include <QApplication>

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
	createInfoPanel();
	verticalSplitter = new QSplitter();
	verticalSplitter->setOrientation(Qt::Orientation::Vertical);
	verticalSplitter->addWidget(horizontalSplitter);
	mainWidget->layout()->addWidget(verticalSplitter);
	createResourceSplitter();
	verticalSplitter->setSizes({ 520, 200 });
	createLogBar();
}

void MainWindow::updateTitle() {
	std::string title = "";
	if (windowTitle()[0] == '*') {
		title += '*';
	}
	title += "NutshellEngine - " + m_globalInfo.projectName;
	if (!m_globalInfo.currentScenePath.empty()) {
		title += " - " + AssetHelper::absoluteToRelative(m_globalInfo.currentScenePath, m_globalInfo.projectDirectory);
	}
	setWindowTitle(QString::fromStdString(title));
}

void MainWindow::createMenuBar() {
	fileMenu = new FileMenu(m_globalInfo);
	menuBar()->addMenu(fileMenu);
	editMenu = new EditMenu(m_globalInfo);
	menuBar()->addMenu(editMenu);
	viewMenu = new ViewMenu(m_globalInfo);
	menuBar()->addMenu(viewMenu);
	projectMenu = new ProjectMenu(m_globalInfo);
	menuBar()->addMenu(projectMenu);
	optionsMenu = new OptionsMenu(m_globalInfo);
	menuBar()->addMenu(optionsMenu);
	helpMenu = new HelpMenu(m_globalInfo);
	menuBar()->addMenu(helpMenu);
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

void MainWindow::createInfoPanel() {
	infoPanel = new InfoPanel(m_globalInfo);
	horizontalSplitter->addWidget(infoPanel);
}

void MainWindow::createResourceSplitter() {
	resourceSplitter = new ResourceSplitter(m_globalInfo);
	verticalSplitter->addWidget(resourceSplitter);
}

void MainWindow::createLogBar() {
	logBar = new LogBar(m_globalInfo);
	mainWidget->layout()->addWidget(logBar);
}

void MainWindow::onCloseSceneConfirmed() {
	m_forceClose = true;
	close();
}

void MainWindow::closeEvent(QCloseEvent* event) {
	if (m_forceClose) {
		event->accept();
		QApplication::closeAllWindows();
		return;
	}

	if (windowTitle()[0] == '*') {
		event->ignore();
		CloseSceneWidget* closeSceneWidget = new CloseSceneWidget(m_globalInfo);
		closeSceneWidget->show();

		connect(closeSceneWidget, &CloseSceneWidget::confirmSignal, this, &MainWindow::onCloseSceneConfirmed);
	}
	else {
		QApplication::closeAllWindows();
	}
}