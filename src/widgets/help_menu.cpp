#include "help_menu.h"
#include <QDesktopServices>
#include <QUrl>
#include <QMessageBox>

HelpMenu::HelpMenu(GlobalInfo& globalInfo) : QMenu("&" + QString::fromStdString(globalInfo.localization.getString("header_help"))), m_globalInfo(globalInfo) {
	m_openDocumentationAction = addAction(QString::fromStdString(m_globalInfo.localization.getString("header_open_documentation")), this, &HelpMenu::openDocumentation);
	addSeparator();
	m_aboutNutshellEngineAction = addAction(QString::fromStdString(m_globalInfo.localization.getString("header_about_nutshell_engine")), this, &HelpMenu::aboutNutshellEngine);
}

void HelpMenu::openDocumentation() {
	QDesktopServices::openUrl(QUrl("https://www.team-nutshell.dev/nutshellengine-docs/"));
}

void HelpMenu::aboutNutshellEngine() {
	if (!m_aboutWidget) {
		m_aboutWidget = new AboutWidget(m_globalInfo);
		m_aboutWidget->show();

		connect(m_aboutWidget, &AboutWidget::closeWindow, this, &HelpMenu::onAboutWidgetClose);
	}
	else {
		m_aboutWidget->activateWindow();
	}
}

void HelpMenu::onAboutWidgetClose() {
	m_aboutWidget = nullptr;
}