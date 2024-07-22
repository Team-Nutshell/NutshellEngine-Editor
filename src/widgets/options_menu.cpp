#include "options_menu.h"
#include "editor_parameters_widget.h"

OptionsMenu::OptionsMenu(GlobalInfo& globalInfo) : QMenu("&Options"), m_globalInfo(globalInfo) {
	m_openEditorParametersAction = addAction("Open Editor Parameters", this, &OptionsMenu::openEditorParameters);
}

void OptionsMenu::openEditorParameters() {
	EditorParametersWidget* editorParametersWidget = new EditorParametersWidget(m_globalInfo);
	editorParametersWidget->show();
}
