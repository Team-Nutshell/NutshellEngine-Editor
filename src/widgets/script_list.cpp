#include "script_list.h"
#include "new_script_message_box.h"
#include "delete_script_widget.h"
#include "main_window.h"
#include <QSizePolicy>
#include <QSignalBlocker>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <regex>

ScriptList::ScriptList(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	if (!std::filesystem::exists(m_globalInfo.projectDirectory + "/scripts/")) {
		return;
	}
	m_scriptsDirectory = std::filesystem::canonical(m_globalInfo.projectDirectory + "/scripts/").string();
	std::replace(m_scriptsDirectory.begin(), m_scriptsDirectory.end(), '\\', '/');

	setWrapping(true);
	setAcceptDrops(true);
	setDragDropMode(QListWidget::DragDrop);
	setResizeMode(QListWidget::Adjust);
	menu = new ScriptListMenu(m_globalInfo);
	menu->scriptList = this;
	setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);

	QSizePolicy sizePolicy;
	sizePolicy.setHorizontalPolicy(QSizePolicy::Policy::Ignored);
	sizePolicy.setVerticalPolicy(QSizePolicy::Policy::Expanding);
	setSizePolicy(sizePolicy);

	updateScriptList();

	if (std::filesystem::exists(m_globalInfo.projectDirectory + "/scripts/")) {
		m_directoryWatcher.addPath(QString::fromStdString(m_scriptsDirectory));
	}

	connect(this, &QListWidget::customContextMenuRequested, this, &ScriptList::showMenu);
	connect(this, &QListWidget::itemDoubleClicked, this, &ScriptList::onItemDoubleClicked);
	connect(&m_directoryWatcher, &QFileSystemWatcher::directoryChanged, this, &ScriptList::onDirectoryChanged);
	connect(itemDelegate(), &QAbstractItemDelegate::closeEditor, this, &ScriptList::onLineEditClose);
}

void ScriptList::newScript() {
	NewScriptMessageBox newScriptMessageBox(m_globalInfo);
	if (newScriptMessageBox.exec() == QMessageBox::StandardButton::Ok) {
		std::string newScriptName = newScriptMessageBox.scriptNameLineEdit->text().toStdString();
		newScriptName.erase(newScriptName.begin(), std::find_if(newScriptName.begin(), newScriptName.end(), [](unsigned char c) {
			return !std::isspace(c);
			}));
		newScriptName.erase(std::find_if(newScriptName.rbegin(), newScriptName.rend(), [](unsigned char c) {
			return !std::isspace(c);
			}).base(), newScriptName.end());

		if (!newScriptName.empty()) {
			if (!std::filesystem::exists(m_globalInfo.projectDirectory + "/scripts/")) {
				std::filesystem::create_directory(m_globalInfo.projectDirectory + "/scripts/");
			}

			const std::regex validFilenameRegex(R"(^[a-zA-Z0-9._ -]+$)");
			if (std::regex_search(newScriptName, validFilenameRegex)) {
				std::string finalScriptName = newScriptName;
				if (std::filesystem::exists(m_globalInfo.projectDirectory + "/scripts/" + newScriptName + ".h")) {
					uint32_t scriptNameIndex = 0;
					finalScriptName = newScriptName + "_" + std::to_string(scriptNameIndex);
					while (std::filesystem::exists(m_globalInfo.projectDirectory + "/scripts/" + finalScriptName + ".h")) {
						scriptNameIndex++;
						finalScriptName = newScriptName + "_" + std::to_string(scriptNameIndex);
					}
					m_globalInfo.logger.addLog(LogLevel::Warning, m_globalInfo.localization.getString("log_script_name_already_taken", { newScriptName, finalScriptName }));
				}

				std::fstream newScriptFile(m_globalInfo.projectDirectory + "/scripts/" + finalScriptName + ".h", std::ios::out | std::ios::trunc);
				newScriptFile << "#pragma once\n#include \"../Common/script/ntshengn_script.h\"\n\nusing namespace NtshEngn;\nstruct " << finalScriptName << " : public Script {\n\tNTSHENGN_SCRIPT(" << finalScriptName << ");\n\n\tvoid init() {\n\n\t}\n\n\tvoid update(float dt) {\n\t\tNTSHENGN_UNUSED(dt);\n\t}\n\n\tvoid destroy() {\n\n\t}\n};";
			}
			else {
				m_globalInfo.logger.addLog(LogLevel::Warning, m_globalInfo.localization.getString("log_script_name_not_valid", { newScriptName }));
			}
		}
	}
}

bool ScriptList::renameScriptFile(const std::string& oldScriptName, const std::string& newScriptName) {
	if (oldScriptName != newScriptName) {
		const std::regex validFilenameRegex(R"(^[a-zA-Z0-9._ -]+$)");
		if (!std::regex_search(newScriptName, validFilenameRegex)) {
			return false;
		}

		if (newScriptName.empty()) {
			return false;
		}

		if (std::filesystem::exists(m_scriptsDirectory + "/" + newScriptName + ".h")) {
			return false;
		}

		if (std::filesystem::exists(m_scriptsDirectory + "/" + oldScriptName + ".h")) {
			std::filesystem::rename(m_scriptsDirectory + "/" + oldScriptName + ".h", m_scriptsDirectory + "/" + newScriptName + ".h");
		}
	}

	renameScriptClass(oldScriptName, newScriptName);

	return true;
}

void ScriptList::renameScriptClass(const std::string& oldScriptName, const std::string& newScriptName) {
	std::ifstream scriptRead(m_globalInfo.projectDirectory + "/scripts/" + newScriptName + ".h");
	std::stringstream scriptStringStream;
	scriptStringStream << scriptRead.rdbuf();
	std::string scriptString = scriptStringStream.str();
	scriptRead.close();

	// Write a temporary string instead of the old script name
	std::string scriptTmp = "${SCRIPT_TMP_STRING}";

	size_t scriptNamePos;
	while ((scriptNamePos = scriptString.find(oldScriptName)) != std::string::npos) {
		scriptString.replace(scriptNamePos, oldScriptName.length(), scriptTmp);
	}

	// Write the new script name instead of the temporary string
	while ((scriptNamePos = scriptString.find(scriptTmp)) != std::string::npos) {
		scriptString.replace(scriptNamePos, scriptTmp.length(), newScriptName);
	}

	std::ofstream scriptWrite(m_globalInfo.projectDirectory + "/scripts/" + newScriptName + ".h", std::ios::trunc);
	scriptWrite << scriptString;
	scriptWrite.close();
}

void ScriptList::deleteScript(const std::string& scriptName) {
	DeleteScriptWidget* deleteScriptWidget = new DeleteScriptWidget(m_globalInfo, scriptName);
	deleteScriptWidget->show();
}

void ScriptList::duplicateScript(const std::string& scriptName) {
	uint32_t scriptNameIndex = 0;
	std::string duplicatedScriptName = scriptName + "_" + std::to_string(scriptNameIndex);
	while (std::filesystem::exists(m_scriptsDirectory + "/" + duplicatedScriptName + ".h")) {
		scriptNameIndex++;
		duplicatedScriptName = scriptName + "_" + std::to_string(scriptNameIndex);
	}
	std::filesystem::copy(m_scriptsDirectory + "/" + scriptName + ".h", m_scriptsDirectory + "/" + duplicatedScriptName + ".h", std::filesystem::copy_options::none);

	renameScriptClass(scriptName, duplicatedScriptName);
}

void ScriptList::updateScriptList() {
	{
		const QSignalBlocker signalBlocker(this);
		clear();
	}

	std::vector<std::string> directoryNames;
	std::vector<std::string> scriptNames;
	for (const auto& entry : std::filesystem::directory_iterator(m_scriptsDirectory)) {
		std::string entryPath = entry.path().string();
		std::replace(entryPath.begin(), entryPath.end(), '\\', '/');

		if (!std::filesystem::is_directory(entry)) {
			std::string scriptPath = entryPath.substr(entryPath.find_last_of('/') + 1);
			scriptNames.push_back(scriptPath.substr(0, scriptPath.find_last_of(".h") - 1));
		}
	}

	std::sort(scriptNames.begin(), scriptNames.end());

	for (const std::string& scriptName : scriptNames) {
		addItem(QString::fromStdString(scriptName));
	}
}

void ScriptList::openCodeEditor(const std::string& scriptName) {
	std::string codeEditorCommand = m_globalInfo.editorParameters.code.codeEditorCommand;

	if (codeEditorCommand.empty()) {
		m_globalInfo.logger.addLog(LogLevel::Warning, m_globalInfo.localization.getString("log_code_editor_none"));

		return;
	}

	std::string scriptPath = m_scriptsDirectory + "/" + scriptName + ".h";
	if (!std::filesystem::exists(scriptPath)) {
		return;
	}

	std::string filePathTemplate = "${FILE_PATH}";
	size_t filePathTemplatePos = codeEditorCommand.find(filePathTemplate);
	if (filePathTemplatePos != std::string::npos) {
		codeEditorCommand.replace(filePathTemplatePos, filePathTemplate.length(), scriptPath);
	}

	std::system(codeEditorCommand.c_str());
}

void ScriptList::onItemDoubleClicked(QListWidgetItem* listWidgetItem) {
	openCodeEditor(listWidgetItem->text().toStdString());
}

void ScriptList::onDirectoryChanged() {
	updateScriptList();
}

void ScriptList::showMenu(const QPoint& pos) {
	QListWidgetItem* item = itemAt(pos);
	if (!item) {
		menu->scriptName = "";
		menu->renameAction->setEnabled(false);
		menu->deleteAction->setEnabled(false);
		menu->duplicateAction->setEnabled(false);
		menu->copyNameAction->setEnabled(false);
	}
	else {
		menu->scriptName = item->text().toStdString();
		menu->renameAction->setEnabled(true);
		menu->deleteAction->setEnabled(true);
		menu->duplicateAction->setEnabled(true);
		menu->copyNameAction->setEnabled(true);
	}
	menu->popup(QCursor::pos());
}

void ScriptList::keyPressEvent(QKeyEvent* event) {
	if (!selectedItems().empty()) {
		QListWidgetItem* listItem = selectedItems()[0];
		int currentSelectionIndex = row(listItem);
		if (event->key() == Qt::Key_Up) {
			clearSelection();
			if (currentSelectionIndex == 0) {
				setCurrentItem(item(count() - 1));
			}
			else {
				setCurrentItem(item(currentSelectionIndex - 1));
			}
		}
		else if (event->key() == Qt::Key_Down) {
			clearSelection();
			if (currentSelectionIndex == (count() - 1)) {
				setCurrentItem(item(0));
			}
			else {
				setCurrentItem(item(currentSelectionIndex + 1));
			}
		}
		else if ((event->key() == Qt::Key_Return) || (event->key() == Qt::Key_Enter)) {
			if (!currentlyEditedItemName.empty()) {
				return;
			}

			openCodeEditor(listItem->text().toStdString());
		}
		else if (event->key() == Qt::Key_F2) {
			if (listItem && (listItem->text() != "../")) {
				currentlyEditedItemName = listItem->text().toStdString();
				listItem->setFlags(listItem->flags() | Qt::ItemFlag::ItemIsEditable);
				editItem(listItem);
			}
		}
		else if (event->key() == Qt::Key_Delete) {
			deleteScript(listItem->text().toStdString());
		}
		else if ((QGuiApplication::keyboardModifiers() == Qt::ControlModifier) && (event->key() == Qt::Key_D)) {
			duplicateScript(listItem->text().toStdString());
		}
	}
}

void ScriptList::onLineEditClose(QWidget* lineEdit, QAbstractItemDelegate::EndEditHint hint) {
	(void)hint;
	QListWidgetItem* currentItem = selectedItems()[0];
	currentItem->setFlags(currentItem->flags() & ~Qt::ItemFlag::ItemIsEditable);
	std::string newName = reinterpret_cast<QLineEdit*>(lineEdit)->text().toStdString();

	if (!renameScriptFile(currentlyEditedItemName, newName)) {
		currentItem->setText(QString::fromStdString(currentlyEditedItemName));
	}

	currentlyEditedItemName = "";
}
