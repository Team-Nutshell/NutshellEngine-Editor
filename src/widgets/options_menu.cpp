#include "options_menu.h"
#include "../undo_commands/select_asset_entities_command.h"
#include <filesystem>

OptionsMenu::OptionsMenu(GlobalInfo& globalInfo) : QMenu("&" + QString::fromStdString(globalInfo.localization.getString("header_options"))), m_globalInfo(globalInfo) {
	m_openEditorParametersAction = addAction(QString::fromStdString(m_globalInfo.localization.getString("header_options_open_editor_parameters")), this, &OptionsMenu::openEditorParameters);
	addSeparator();
	m_openProjectOptionsAction = addAction(QString::fromStdString(m_globalInfo.localization.getString("header_options_open_project_options")), this, &OptionsMenu::openProjectOptions);
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

void OptionsMenu::openProjectOptions() {
	const std::string optionsFilePath = m_globalInfo.projectDirectory + "/assets/options/options.ntop";
	if (std::filesystem::exists(optionsFilePath)) {
		m_globalInfo.selectionUndoStack->push(new SelectAssetEntitiesCommand(m_globalInfo, SelectionType::Asset, optionsFilePath, NO_ENTITY, {}));
	}
}

void OptionsMenu::onEditorParametersWidgetClose() {
	m_editorParametersWidget = nullptr;
}
