#include "scriptable_component_widget.h"
#include "component_title_widget.h"
#include "separator_line.h"
#include "new_script_message_box.h"
#include "main_window.h"
#include "../common/save_title_changer.h"
#include "../undo_commands/change_entities_component_command.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSignalBlocker>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <cstdlib>

ScriptableComponentWidget::ScriptableComponentWidget(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setLayout(new QVBoxLayout());
	layout()->setAlignment(Qt::AlignmentFlag::AlignTop);
	layout()->setContentsMargins(0, 0, 0, 0);
	layout()->addWidget(new ComponentTitleWidget(m_globalInfo, "Scriptable"));
	QWidget* scriptSelectorWidget = new QWidget();
	scriptSelectorWidget->setLayout(new QHBoxLayout());
	scriptSelectorWidget->layout()->setContentsMargins(0, 0, 0, 0);
	std::vector<std::string> scriptEntries = getScriptEntries();
	scriptNameWidget = new ComboBoxWidget(m_globalInfo, "Script", scriptEntries);
	scriptSelectorWidget->layout()->addWidget(scriptNameWidget);
	openCodeEditorButton = new QPushButton("E");
	openCodeEditorButton->setFixedWidth(20);
	scriptSelectorWidget->layout()->addWidget(openCodeEditorButton);
	layout()->addWidget(scriptSelectorWidget);
	layout()->addWidget(new SeparatorLine(m_globalInfo));

	connect(scriptNameWidget, &ComboBoxWidget::elementSelected, this, &ScriptableComponentWidget::onElementChanged);
	connect(openCodeEditorButton, &QPushButton::clicked, this, &ScriptableComponentWidget::onOpenCodeEditorButtonClicked);
	connect(&globalInfo.signalEmitter, &SignalEmitter::selectEntitySignal, this, &ScriptableComponentWidget::onEntitySelected);
	connect(&globalInfo.signalEmitter, &SignalEmitter::addEntityScriptableSignal, this, &ScriptableComponentWidget::onEntityScriptableAdded);
	connect(&globalInfo.signalEmitter, &SignalEmitter::removeEntityScriptableSignal, this, &ScriptableComponentWidget::onEntityScriptableRemoved);
	connect(&globalInfo.signalEmitter, &SignalEmitter::changeEntityScriptableSignal, this, &ScriptableComponentWidget::onEntityScriptableChanged);
	connect(&m_scriptsDirectoryWatcher, &QFileSystemWatcher::directoryChanged, this, &ScriptableComponentWidget::onDirectoryChanged);
}

void ScriptableComponentWidget::updateWidgets(const Scriptable& scriptable) {
	if (!scriptable.scriptName.empty()) {
		{
			const QSignalBlocker signalBlocker(scriptNameWidget->comboBox);
			if (scriptNameWidget->comboBox->findText(QString::fromStdString(scriptable.scriptName)) != -1) {
				scriptNameWidget->comboBox->setCurrentText(QString::fromStdString(scriptable.scriptName));
				openCodeEditorButton->setEnabled(true);
			}
			else {
				scriptNameWidget->comboBox->addItem(QString::fromStdString(scriptable.scriptName));
				scriptNameWidget->comboBox->setCurrentText(QString::fromStdString(scriptable.scriptName));
				openCodeEditorButton->setEnabled(false);
			}
		}
	}
	else {
		{
			const QSignalBlocker signalBlocker(scriptNameWidget->comboBox);
			scriptNameWidget->comboBox->setCurrentText("No script selected");
		}
		openCodeEditorButton->setEnabled(false);
	}
}

void ScriptableComponentWidget::updateComponent(EntityID entityID, Component* component) {
	m_globalInfo.undoStack->push(new ChangeEntitiesComponentCommand(m_globalInfo, { entityID }, "Scriptable", { component }));
}

std::vector<std::string> ScriptableComponentWidget::getScriptEntries() {
	m_scriptToPath.clear();
	std::vector<std::string> scriptEntries;
	scriptEntries.push_back("No script selected");
	if (std::filesystem::exists(m_globalInfo.projectDirectory + "/scripts/")) {
		for (const auto& entry : std::filesystem::directory_iterator(m_globalInfo.projectDirectory + "/scripts/")) {
			if (entry.is_directory()) {
				continue;
			}

			std::fstream scriptFile(entry.path().string(), std::ios::in);
			if (scriptFile.is_open()) {
				std::string scriptContent((std::istreambuf_iterator<char>(scriptFile)), std::istreambuf_iterator<char>());
				size_t scriptNameStartPos = scriptContent.find("NTSHENGN_SCRIPT(");
				if (scriptNameStartPos != std::string::npos) {
					scriptNameStartPos += 16;
					size_t scriptNameLength = scriptContent.substr(scriptNameStartPos).find(")");
					if (scriptNameLength != std::string::npos) {
						std::string scriptName = scriptContent.substr(scriptNameStartPos, scriptNameLength);
						scriptEntries.push_back(scriptName);
						m_scriptToPath[scriptName] = entry.path().string();
					}
				}
			}
		}
		m_scriptsDirectoryWatcher.addPath(QString::fromStdString(m_globalInfo.projectDirectory) + "/scripts/");
	}
	scriptEntries.push_back("+ New script...");

	return scriptEntries;
}

void ScriptableComponentWidget::onEntitySelected() {
	if ((m_globalInfo.currentEntityID != NO_ENTITY) && m_globalInfo.entities[m_globalInfo.currentEntityID].scriptable.has_value()) {
		show();
		updateWidgets(m_globalInfo.entities[m_globalInfo.currentEntityID].scriptable.value());
	}
	else {
		hide();
	}
}

void ScriptableComponentWidget::onEntityScriptableAdded(EntityID entityID) {
	if (entityID == m_globalInfo.currentEntityID) {
		const Scriptable& scriptable = m_globalInfo.entities[m_globalInfo.currentEntityID].scriptable.value();
		updateWidgets(scriptable);
		show();
	}

	SaveTitleChanger::change(m_globalInfo.mainWindow);
}

void ScriptableComponentWidget::onEntityScriptableRemoved(EntityID entityID) {
	if (entityID == m_globalInfo.currentEntityID) {
		hide();
	}

	SaveTitleChanger::change(m_globalInfo.mainWindow);
}

void ScriptableComponentWidget::onEntityScriptableChanged(EntityID entityID, const Scriptable& scriptable) {
	QObject* senderWidget = sender();
	if (senderWidget != this) {
		if (entityID == m_globalInfo.currentEntityID) {
			updateWidgets(scriptable);
		}
	}

	SaveTitleChanger::change(m_globalInfo.mainWindow);
}

void ScriptableComponentWidget::onElementChanged(const std::string& element) {
	Scriptable newScriptable = m_globalInfo.entities[m_globalInfo.currentEntityID].scriptable.value();

	QObject* senderWidget = sender();
	if (senderWidget == scriptNameWidget) {
		if (element == "No script selected") {
			newScriptable.scriptName = "";
		}
		else if (element == "+ New script...") {
			NewScriptMessageBox newScriptMessageBox(m_globalInfo);
			if (newScriptMessageBox.exec() == QMessageBox::StandardButton::Ok) {
				std::string scriptName = newScriptMessageBox.scriptNameLineEdit->text().toStdString();
				scriptName.erase(scriptName.begin(), std::find_if(scriptName.begin(), scriptName.end(), [](unsigned char c) {
					return !std::isspace(c);
					}));
				scriptName.erase(std::find_if(scriptName.rbegin(), scriptName.rend(), [](unsigned char c) {
					return !std::isspace(c);
					}).base(), scriptName.end());

				if (!scriptName.empty()) {
					if (!std::filesystem::exists(m_globalInfo.projectDirectory + "/scripts/")) {
						std::filesystem::create_directory(m_globalInfo.projectDirectory + "/scripts/");
						m_scriptsDirectoryWatcher.addPath(QString::fromStdString(m_globalInfo.projectDirectory) + "/scripts/");
					}

					std::fstream newScriptFile(m_globalInfo.projectDirectory + "/scripts/" + scriptName + ".h", std::ios::out | std::ios::trunc);
					newScriptFile << "#pragma once\n#include \"../Common/script/ntshengn_script.h\"\n\nusing namespace NtshEngn;\nstruct " << scriptName << " : public Script {\n\tNTSHENGN_SCRIPT(" << scriptName << ");\n\n\tvoid init() {\n\n\t}\n\n\tvoid update(double dt) {\n\t\tNTSHENGN_UNUSED(dt);\n\t}\n\n\tvoid destroy() {\n\n\t}\n};";
				
					newScriptable.scriptName = scriptName;
				}
			}

		}
		else {
			newScriptable.scriptName = element;
		}
	}
	updateComponent(m_globalInfo.currentEntityID, &newScriptable);
}

void ScriptableComponentWidget::onOpenCodeEditorButtonClicked() {
	std::string codeEditorCommand = m_globalInfo.editorParameters.code.codeEditorCommand;

	if (codeEditorCommand.empty()) {
		m_globalInfo.logger.addLog(LogLevel::Warning, "No code editor command has been specified.");

		return;
	}

	std::string currentText = scriptNameWidget->comboBox->currentText().toStdString();
	std::string scriptPath = "";
	if (m_scriptToPath.find(currentText) == m_scriptToPath.end()) {
		return;
	}
	scriptPath = m_scriptToPath[currentText];

	std::string filePathTemplate = "${FILE_PATH}";
	size_t filePathTemplatePos = codeEditorCommand.find(filePathTemplate);
	if (filePathTemplatePos != std::string::npos) {
		codeEditorCommand.replace(filePathTemplatePos, filePathTemplate.length(), scriptPath);
	}

	std::system(codeEditorCommand.c_str());
}

void ScriptableComponentWidget::onDirectoryChanged(const QString& path) {
	(void)path;
	std::vector<std::string> scriptEntries = getScriptEntries();

	{
		const QSignalBlocker signalBlocker(scriptNameWidget->comboBox);
		scriptNameWidget->comboBox->clear();
		for (auto const& scriptEntry : scriptEntries) {
			scriptNameWidget->comboBox->addItem(QString::fromStdString(scriptEntry));
		}
	}

	if ((m_globalInfo.currentEntityID != NO_ENTITY) && m_globalInfo.entities[m_globalInfo.currentEntityID].scriptable) {
		updateWidgets(m_globalInfo.entities[m_globalInfo.currentEntityID].scriptable.value());
	}
}
