#include "scriptable_component_widget.h"
#include "component_title_widget.h"
#include "separator_line.h"
#include "new_script_message_box.h"
#include "../undo_commands/change_entity_component_command.h"
#include <QVBoxLayout>
#include <QSignalBlocker>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>

ScriptableComponentWidget::ScriptableComponentWidget(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setLayout(new QVBoxLayout());
	layout()->setAlignment(Qt::AlignmentFlag::AlignTop);
	layout()->setContentsMargins(0, 0, 0, 0);
	layout()->addWidget(new ComponentTitleWidget(m_globalInfo, "Scriptable"));
	std::vector<std::string> scriptEntries;
	scriptEntries.push_back("No script selected");
	if (std::filesystem::exists(m_globalInfo.projectDirectory + "/scripts/")) {
		for (const auto& entry : std::filesystem::directory_iterator(m_globalInfo.projectDirectory + "/scripts/")) {
			if (entry.path().string().find("generate_scriptable_factory.py") != std::string::npos) {
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
						scriptEntries.push_back(scriptContent.substr(scriptNameStartPos, scriptNameLength));
					}
				}
			}
		}
		m_scriptsDirectoryWatcher.addPath(QString::fromStdString(m_globalInfo.projectDirectory) + "/scripts/");
	}
	scriptEntries.push_back("+ New script...");
	scriptNameWidget = new ComboBoxWidget(m_globalInfo, "Script", scriptEntries);
	layout()->addWidget(scriptNameWidget);
	layout()->addWidget(new SeparatorLine(m_globalInfo));

	connect(scriptNameWidget, &ComboBoxWidget::elementSelected, this, &ScriptableComponentWidget::onElementUpdated);
	connect(&globalInfo.signalEmitter, &SignalEmitter::selectEntitySignal, this, &ScriptableComponentWidget::onSelectEntity);
	connect(&globalInfo.signalEmitter, &SignalEmitter::addEntityScriptableSignal, this, &ScriptableComponentWidget::onAddEntityScriptable);
	connect(&globalInfo.signalEmitter, &SignalEmitter::removeEntityScriptableSignal, this, &ScriptableComponentWidget::onRemoveEntityScriptable);
	connect(&globalInfo.signalEmitter, &SignalEmitter::changeEntityScriptableSignal, this, &ScriptableComponentWidget::onChangeEntityScriptable);
	connect(&m_scriptsDirectoryWatcher, &QFileSystemWatcher::directoryChanged, this, &ScriptableComponentWidget::onDirectoryChanged);
}

void ScriptableComponentWidget::updateWidgets(const Scriptable& scriptable) {
	if (scriptable.scriptName != "") {
		{
			const QSignalBlocker signalBlocker(scriptNameWidget->comboBox);
			if (scriptNameWidget->comboBox->findText(QString::fromStdString(scriptable.scriptName)) != -1) {
				scriptNameWidget->comboBox->setCurrentText(QString::fromStdString(scriptable.scriptName));
			}
			else {
				scriptNameWidget->comboBox->addItem(QString::fromStdString(scriptable.scriptName));
				scriptNameWidget->comboBox->setCurrentText(QString::fromStdString(scriptable.scriptName));
			}
		}
	}
	else {
		{
			const QSignalBlocker signalBlocker(scriptNameWidget->comboBox);
			scriptNameWidget->comboBox->setCurrentText("No script selected");
		}
	}
}

void ScriptableComponentWidget::onSelectEntity() {
	if ((m_globalInfo.currentEntityID != NO_ENTITY) && m_globalInfo.entities[m_globalInfo.currentEntityID].scriptable.has_value()) {
		show();
		updateWidgets(m_globalInfo.entities[m_globalInfo.currentEntityID].scriptable.value());
	}
	else {
		hide();
	}
}

void ScriptableComponentWidget::onAddEntityScriptable(EntityID entityID) {
	if (entityID == m_globalInfo.currentEntityID) {
		Scriptable scriptable = m_globalInfo.entities[m_globalInfo.currentEntityID].scriptable.value();
		updateWidgets(scriptable);
		show();
	}
}

void ScriptableComponentWidget::onRemoveEntityScriptable(EntityID entityID) {
	if (entityID == m_globalInfo.currentEntityID) {
		hide();
	}
}

void ScriptableComponentWidget::onChangeEntityScriptable(EntityID entityID, const Scriptable& scriptable) {
	if (sender() != this) {
		if (entityID == m_globalInfo.currentEntityID) {
			updateWidgets(scriptable);
		}
	}
}

void ScriptableComponentWidget::onElementUpdated(const std::string& element) {
	Scriptable newScriptable = m_globalInfo.entities[m_globalInfo.currentEntityID].scriptable.value();
	if (sender() == scriptNameWidget) {
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

				if (scriptName != "") {
					if (!std::filesystem::exists(m_globalInfo.projectDirectory + "/scripts/")) {
						std::filesystem::create_directory(m_globalInfo.projectDirectory + "/scripts/");
						m_scriptsDirectoryWatcher.addPath(QString::fromStdString(m_globalInfo.projectDirectory) + "/scripts/");
					}

					std::fstream newScriptFile(m_globalInfo.projectDirectory + "/scripts/" + scriptName + ".h", std::ios::out | std::ios::trunc);
					newScriptFile << "#pragma once\n#include \"../Core/scripting/ntshengn_script.h\n\nusing namespace NtshEngn;\nstruct " << scriptName << " : public Script {\n\tNTSHENGN_SCRIPT(" << scriptName << ");\n\n\tvoid init() {\n\n\t}\n\n\tvoid update(double dt) {\n\t\tNTSHENGN_UNUSED(dt);\n\t}\n\n\tvoid destroy() {\n\n\t}\t\n};";
				
					newScriptable.scriptName = scriptName;
				}
			}

		}
		else {
			newScriptable.scriptName = element;
		}
	}
	m_globalInfo.undoStack->push(new ChangeEntityComponentCommand(m_globalInfo, m_globalInfo.currentEntityID, "Scriptable", &newScriptable));
}

void ScriptableComponentWidget::onDirectoryChanged(const QString& path) {
	(void)path;
	std::vector<std::string> scriptEntries;
	scriptEntries.push_back("No script selected");
	if (std::filesystem::exists(m_globalInfo.projectDirectory + "/scripts/")) {
		for (const auto& entry : std::filesystem::directory_iterator(m_globalInfo.projectDirectory + "/scripts/")) {
			if (entry.path().string().find("generate_scriptable_factory.py") != std::string::npos) {
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
						scriptEntries.push_back(scriptContent.substr(scriptNameStartPos, scriptNameLength));
					}
				}
			}
		}
	}
	scriptEntries.push_back("+ New script...");

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
