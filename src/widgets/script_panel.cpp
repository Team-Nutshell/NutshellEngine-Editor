#include "script_panel.h"
#include <QVBoxLayout>

ScriptPanel::ScriptPanel(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setLayout(new QVBoxLayout());
	layout()->setContentsMargins(0, 2, 2, 0);
	scriptsLabel = new QLabel(QString::fromStdString(m_globalInfo.localization.getString("scripts_list_scripts")));
	if (!std::filesystem::exists(m_globalInfo.projectDirectory + "/scripts/")) {
		scriptsLabel->setText(QString::fromStdString(m_globalInfo.localization.getString("scripts_list_scripts")) + " (<i>" + QString::fromStdString(m_globalInfo.localization.getString("scripts_list_scripts_directory_missing")) + "</i>)");
	}
	layout()->addWidget(scriptsLabel);
	scriptList = new ScriptList(m_globalInfo);
	layout()->addWidget(scriptList);
}