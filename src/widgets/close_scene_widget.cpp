#include "close_scene_widget.h"
#include "../common/scene_manager.h"
#include <QVBoxLayout>
#include <QLabel>

CloseSceneWidget::CloseSceneWidget(GlobalInfo& globalInfo) : m_globalInfo(globalInfo), m_scenePath(globalInfo.currentScenePath) {
	setFixedWidth(350);
	setFixedHeight(125);
	setWindowTitle("NutshellEngine - " + QString::fromStdString(m_globalInfo.localization.getString("close_scene")));
	setWindowIcon(QIcon("assets/icon.png"));
	setAttribute(Qt::WA_DeleteOnClose);
	setModal(true);

	setLayout(new QVBoxLayout());
	QLabel* closeSceneLabel = new QLabel(QString::fromStdString(m_globalInfo.localization.getString("unsaved_scene_changes", { m_scenePath })));
	closeSceneLabel->setWordWrap(true);
	layout()->addWidget(closeSceneLabel);
	QWidget* buttonsWidget = new QWidget();
	buttonsWidget->setLayout(new QHBoxLayout());
	m_saveSceneButton = new QPushButton(QString::fromStdString(m_globalInfo.localization.getString("save")));
	buttonsWidget->layout()->addWidget(m_saveSceneButton);
	m_dontSaveSceneButton = new QPushButton(QString::fromStdString(m_globalInfo.localization.getString("dont_save")));
	buttonsWidget->layout()->addWidget(m_dontSaveSceneButton);
	m_cancelButton = new QPushButton(QString::fromStdString(m_globalInfo.localization.getString("cancel")));
	buttonsWidget->layout()->addWidget(m_cancelButton);
	layout()->addWidget(buttonsWidget);

	connect(m_saveSceneButton, &QPushButton::clicked, this, &CloseSceneWidget::onSaveSceneButtonClicked);
	connect(m_dontSaveSceneButton, &QPushButton::clicked, this, &CloseSceneWidget::onDontSaveSceneButtonClicked);
	connect(m_cancelButton, &QPushButton::clicked, this, &CloseSceneWidget::onCancelButtonClicked);
}

void CloseSceneWidget::onSaveSceneButtonClicked() {
	SceneManager::saveScene(m_globalInfo, m_scenePath);
	emit confirmSignal();
	close();
}

void CloseSceneWidget::onDontSaveSceneButtonClicked() {
	emit confirmSignal();
	close();
}

void CloseSceneWidget::onCancelButtonClicked() {
	close();
}
