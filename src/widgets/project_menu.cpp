#include "project_menu.h"
#include "project_ntpj_file_widget.h"

ProjectMenu::ProjectMenu(GlobalInfo& globalInfo) : QMenu("&Project"), m_globalInfo(globalInfo) {
	m_openProjectSettingsAction = addAction("Open Project Settings", this, &ProjectMenu::openProjectSettings);
	m_openProjectSettingsAction->setShortcut(QKeySequence::fromString("Ctrl+Shift+P"));
}

void ProjectMenu::openProjectSettings() {
	ProjectNtpjFileWidget* projectNtpjFileWidget = new ProjectNtpjFileWidget(m_globalInfo);
	projectNtpjFileWidget->show();
}
