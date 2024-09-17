#include "options_menu.h"
#include "editor_parameters_widget.h"

OptionsMenu::OptionsMenu(GlobalInfo& globalInfo) : QMenu("&" + QString::fromStdString(globalInfo.localization.getString("header_options"))), m_globalInfo(globalInfo) {
	m_openEditorParametersAction = addAction(QString::fromStdString(m_globalInfo.localization.getString("header_options_open_editor_parameters")), this, &OptionsMenu::openEditorParameters);
}

void OptionsMenu::openEditorParameters() {
	EditorParametersWidget* editorParametersWidget = new EditorParametersWidget(m_globalInfo);
	editorParametersWidget->show();
}
