#include "script_list_menu.h"
#include "script_list.h"
#include "delete_script_widget.h"
#include "main_window.h"
#include <QClipboard>
#include <filesystem>
#include <fstream>

ScriptListMenu::ScriptListMenu(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	newAction = addAction(QString::fromStdString(m_globalInfo.localization.getString("script_new_script")), this, &ScriptListMenu::newScript);
	renameAction = addAction(QString::fromStdString(m_globalInfo.localization.getString("scripts_rename")), this, &ScriptListMenu::renameScript);
	renameAction->setShortcut(QKeySequence::fromString("F2"));
	deleteAction = addAction(QString::fromStdString(m_globalInfo.localization.getString("scripts_delete")), this, &ScriptListMenu::deleteScript);
	deleteAction->setShortcut(QKeySequence::fromString("Del"));
	duplicateAction = addAction(QString::fromStdString(m_globalInfo.localization.getString("scripts_duplicate")), this, &ScriptListMenu::duplicateScript);
	duplicateAction->setShortcut(QKeySequence::fromString("Ctrl+D"));
	addSeparator();
	copyNameAction = addAction(QString::fromStdString(m_globalInfo.localization.getString("scripts_copy_name")), this, &ScriptListMenu::copyName);
	addSeparator();
	openDirectoryInCodeEditorAction = addAction(QString::fromStdString(m_globalInfo.localization.getString("scripts_open_directory_code_editor")), this, &ScriptListMenu::openDirectoryInCodeEditor);
}

void ScriptListMenu::newScript() {
	scriptList->newScript();
}

void ScriptListMenu::renameScript() {
	m_globalInfo.mainWindow->resourceSplitter->scriptPanel->scriptList->currentlyEditedItemName = scriptName;
	QListWidgetItem* item = m_globalInfo.mainWindow->resourceSplitter->scriptPanel->scriptList->selectedItems()[0];
	item->setFlags(item->flags() | Qt::ItemFlag::ItemIsEditable);
	m_globalInfo.mainWindow->resourceSplitter->scriptPanel->scriptList->editItem(item);
}

void ScriptListMenu::deleteScript() {
	scriptList->deleteScript(scriptName);
}

void ScriptListMenu::duplicateScript() {
	scriptList->duplicateScript(scriptName);
}

void ScriptListMenu::copyName() {
	QGuiApplication::clipboard()->setText(QString::fromStdString(scriptName));
}

void ScriptListMenu::openDirectoryInCodeEditor() {
	std::string codeEditorCommand = m_globalInfo.editorParameters.code.codeEditorCommand;

	if (codeEditorCommand.empty()) {
		m_globalInfo.logger.addLog(LogLevel::Warning, m_globalInfo.localization.getString("log_code_editor_none"));

		return;
	}

	std::string filePathTemplate = "${FILE_PATH}";
	size_t filePathTemplatePos = codeEditorCommand.find(filePathTemplate);
	if (filePathTemplatePos != std::string::npos) {
		codeEditorCommand.replace(filePathTemplatePos, filePathTemplate.length(), m_globalInfo.projectDirectory + "/scripts");
	}

	std::system(codeEditorCommand.c_str());
}
