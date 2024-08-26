#include "project_menu.h"
#include "project_ntpj_file_widget.h"

ProjectMenu::ProjectMenu(GlobalInfo& globalInfo) : QMenu("&Project"), m_globalInfo(globalInfo) {
	m_openProjectSettingsAction = addAction("Open Project Settings", this, &ProjectMenu::openProjectSettings);
	m_openProjectSettingsAction->setShortcut(QKeySequence::fromString("Ctrl+Shift+P"));
	addSeparator();
	m_importGlobalResources = addAction("Import global resources", this, &ProjectMenu::importGlobalResources);
	m_updateBaseProject = addAction("Update base project", this, &ProjectMenu::updateBaseProject);
}

void ProjectMenu::openProjectSettings() {
	ProjectNtpjFileWidget* projectNtpjFileWidget = new ProjectNtpjFileWidget(m_globalInfo);
	projectNtpjFileWidget->show();
}

void ProjectMenu::importGlobalResources() {
	std::filesystem::copy("assets/global_resources", m_globalInfo.projectDirectory, std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive);
}

void ProjectMenu::updateBaseProject() {
	std::filesystem::copy("assets/base_project", m_globalInfo.projectDirectory, std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive);
}
