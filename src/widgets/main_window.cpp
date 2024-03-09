#include "main_window.h"
#include <QVBoxLayout>
#include <QMenuBar>
#include <memory>

MainWindow::MainWindow(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	resize(1280, 720);
	setWindowTitle(QString("NutshellEngine - ") + QString::fromStdString(globalInfo.projectName));
	setWindowIcon(QIcon("assets/icon.png"));
	m_mainWidget = std::make_unique<QWidget>();
	m_horizontalSplitter = std::make_unique<QSplitter>();
	m_horizontalSplitter->setOrientation(Qt::Orientation::Horizontal);
	setCentralWidget(m_mainWidget.get());
	createMenuBar();
	createEntityPanel();
	createRendererPanel();
	createEntityInfoPanel();
	m_verticalSplitter = std::make_unique<QSplitter>();
	m_verticalSplitter->setOrientation(Qt::Orientation::Vertical);
	m_verticalSplitter->addWidget(m_horizontalSplitter.get());
	m_mainWidget->setLayout(new QVBoxLayout());
	m_mainWidget->layout()->addWidget(m_verticalSplitter.get());
	createResourcePanel();
	m_verticalSplitter->setSizes({ 520, 200 });
}

void MainWindow::createMenuBar() {
	m_fileMenu = std::make_unique<FileMenu>(m_globalInfo);
	menuBar()->addMenu(m_fileMenu.get());
	m_editMenu = std::make_unique<EditMenu>(m_globalInfo);
	menuBar()->addMenu(m_editMenu.get());
	m_viewMenu = std::make_unique<ViewMenu>(m_globalInfo);
	menuBar()->addMenu(m_viewMenu.get());
}

void MainWindow::createEntityPanel() {
	m_entityPanel = std::make_unique<EntityPanel>(m_globalInfo);
	m_horizontalSplitter->addWidget(m_entityPanel.get());
}

void MainWindow::createRendererPanel() {
}

void MainWindow::createEntityInfoPanel() {
}

void MainWindow::createResourcePanel() {
	m_resourcePanel = std::make_unique<ResourcePanel>(m_globalInfo);
	m_verticalSplitter->addWidget(m_resourcePanel.get());
}
