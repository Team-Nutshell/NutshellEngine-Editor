#include "delete_script_widget.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <filesystem>

DeleteScriptWidget::DeleteScriptWidget(GlobalInfo& globalInfo, const std::string& scriptName) : m_globalInfo(globalInfo), m_scriptName(scriptName) {
	setWindowTitle("NutshellEngine - " + QString::fromStdString(m_globalInfo.localization.getString("scripts_delete")) + " - " + QString::fromStdString(scriptName));
	setWindowIcon(QIcon("assets/icon.png"));
	setAttribute(Qt::WA_DeleteOnClose);
	setModal(true);

	setLayout(new QVBoxLayout());
	layout()->setAlignment(Qt::AlignmentFlag::AlignHCenter);
	layout()->addWidget(new QLabel(QString::fromStdString(m_globalInfo.localization.getString("scripts_confirm_delete", { m_scriptName }))));
	layout()->addWidget(new QLabel("<b>" + QString::fromStdString(m_globalInfo.localization.getString("scripts_delete_cannot_undo")) + "</b>"));
	QWidget* buttonLayoutWidget = new QWidget();
	buttonLayoutWidget->setLayout(new QHBoxLayout());
	okButton = new QPushButton(QString::fromStdString(m_globalInfo.localization.getString("ok", { m_scriptName })));
	buttonLayoutWidget->layout()->addWidget(okButton);
	cancelButton = new QPushButton(QString::fromStdString(m_globalInfo.localization.getString("cancel", { m_scriptName })));
	buttonLayoutWidget->layout()->addWidget(cancelButton);
	layout()->addWidget(buttonLayoutWidget);

	connect(okButton, &QPushButton::clicked, this, &DeleteScriptWidget::onOkButtonClicked);
	connect(cancelButton, &QPushButton::clicked, this, &DeleteScriptWidget::onCancelButtonClicked);
}

void DeleteScriptWidget::onOkButtonClicked() {
	if (std::filesystem::exists(m_globalInfo.projectDirectory + "/scripts/" + m_scriptName + ".h")) {
		std::filesystem::remove(m_globalInfo.projectDirectory + "/scripts/" + m_scriptName + ".h");
	}
	close();
}

void DeleteScriptWidget::onCancelButtonClicked() {
	close();
}

void DeleteScriptWidget::keyPressEvent(QKeyEvent* event) {
	if (event->isAutoRepeat()) {
		event->accept();
		return;
	}

	if (event->key() == Qt::Key_Escape) {
		close();
	}
}