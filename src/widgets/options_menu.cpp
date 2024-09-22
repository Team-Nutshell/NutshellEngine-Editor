#include "options_menu.h"

OptionsMenu::OptionsMenu(GlobalInfo& globalInfo) : QMenu("&" + QString::fromStdString(globalInfo.localization.getString("header_options"))), m_globalInfo(globalInfo) {
	m_openEditorParametersAction = addAction(QString::fromStdString(m_globalInfo.localization.getString("header_options_open_editor_parameters")), this, &OptionsMenu::openEditorParameters);
}

void OptionsMenu::openEditorParameters() {
	if (!m_editorParametersWidget) {
		m_editorParametersWidget = new EditorParametersWidget(m_globalInfo);
		m_editorParametersWidget->show();

		connect(m_editorParametersWidget, &EditorParametersWidget::closeWindow, this, &OptionsMenu::onEditorParametersWidgetClose);
	}
	else {
		m_editorParametersWidget->activateWindow();
	}
}

void OptionsMenu::onEditorParametersWidgetClose() {
	m_editorParametersWidget = nullptr;
}
