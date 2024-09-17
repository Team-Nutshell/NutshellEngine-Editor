#include "new_script_message_box.h"
#include "../common/scene_manager.h"
#include <QGridLayout>
#include <QLabel>

NewScriptMessageBox::NewScriptMessageBox(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setFixedSize(400, 300);
	setWindowTitle("NutshellEngine - " + QString::fromStdString(m_globalInfo.localization.getString("script_new_script")));
	setWindowIcon(QIcon("assets/icon.png"));
	setStandardButtons(QMessageBox::StandardButton::Ok | QMessageBox::StandardButton::Cancel);

	QGridLayout* gridLayout = static_cast<QGridLayout*>(layout());
	gridLayout->addWidget(new QLabel(QString::fromStdString(m_globalInfo.localization.getString("script_new_script_enter_name"))), 0, 0);
	scriptNameLineEdit = new QLineEdit();
	gridLayout->addWidget(scriptNameLineEdit, 1, 0);
}