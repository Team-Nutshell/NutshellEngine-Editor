#include "scriptable_component_widget.h"
#include "component_title_widget.h"
#include "separator_line.h"
#include "new_script_message_box.h"
#include "boolean_widget.h"
#include "integer_widget.h"
#include "scalar_widget.h"
#include "string_widget.h"
#include "vector2_widget.h"
#include "vector3_widget.h"
#include "vector4_widget.h"
#include "quaternion_widget.h"
#include "main_window.h"
#include "../common/save_title_changer.h"
#include "../undo_commands/change_entities_component_command.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSignalBlocker>
#include <fstream>
#include <filesystem>
#include <limits>
#include <regex>
#include <cstdlib>
#include <cstdint>
#include <cctype>

ScriptableComponentWidget::ScriptableComponentWidget(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setLayout(new QVBoxLayout());
	layout()->setAlignment(Qt::AlignmentFlag::AlignTop);
	layout()->setContentsMargins(0, 0, 0, 0);
	layout()->addWidget(new ComponentTitleWidget(m_globalInfo, m_globalInfo.localization.getString("component_scriptable")));
	QWidget* scriptSelectorWidget = new QWidget();
	scriptSelectorWidget->setLayout(new QHBoxLayout());
	scriptSelectorWidget->layout()->setContentsMargins(0, 0, 0, 0);
	std::vector<std::string> scriptEntries = getScriptEntries();
	scriptNameWidget = new ComboBoxWidget(m_globalInfo, m_globalInfo.localization.getString("component_scriptable_script"), scriptEntries);
	scriptSelectorWidget->layout()->addWidget(scriptNameWidget);
	openCodeEditorButton = new QPushButton("E");
	openCodeEditorButton->setFixedWidth(20);
	scriptSelectorWidget->layout()->addWidget(openCodeEditorButton);
	layout()->addWidget(scriptSelectorWidget);
	editableScriptVariablesWidget = new QWidget();
	editableScriptVariablesWidget->setLayout(new QVBoxLayout());
	editableScriptVariablesWidget->layout()->setContentsMargins(0, 0, 0, 0);
	editableScriptVariablesWidget->layout()->addWidget(new QLabel(QString::fromStdString(m_globalInfo.localization.getString("component_scriptable_editable_variables"))));
	layout()->addWidget(editableScriptVariablesWidget);
	layout()->addWidget(new SeparatorLine());

	connect(scriptNameWidget, &ComboBoxWidget::elementSelected, this, &ScriptableComponentWidget::onElementChanged);
	connect(openCodeEditorButton, &QPushButton::clicked, this, &ScriptableComponentWidget::onOpenCodeEditorButtonClicked);
	connect(&globalInfo.signalEmitter, &SignalEmitter::selectEntitySignal, this, &ScriptableComponentWidget::onEntitySelected);
	connect(&globalInfo.signalEmitter, &SignalEmitter::addEntityScriptableSignal, this, &ScriptableComponentWidget::onEntityScriptableAdded);
	connect(&globalInfo.signalEmitter, &SignalEmitter::removeEntityScriptableSignal, this, &ScriptableComponentWidget::onEntityScriptableRemoved);
	connect(&globalInfo.signalEmitter, &SignalEmitter::changeEntityScriptableSignal, this, &ScriptableComponentWidget::onEntityScriptableChanged);
	connect(&m_scriptsDirectoryWatcher, &QFileSystemWatcher::directoryChanged, this, &ScriptableComponentWidget::onDirectoryChanged);
	connect(&m_currentScriptWatcher, &QFileSystemWatcher::fileChanged, this, &ScriptableComponentWidget::onCurrentScriptChanged);
}

void ScriptableComponentWidget::updateWidgets(const Scriptable& scriptable) {
	if (!scriptable.scriptName.empty()) {
		{
			const QSignalBlocker signalBlocker(scriptNameWidget->comboBox);
			if (scriptNameWidget->comboBox->findText(QString::fromStdString(scriptable.scriptName)) != -1) {
				std::string currentScript = scriptNameWidget->comboBox->currentText().toStdString();
				scriptNameWidget->comboBox->setCurrentText(QString::fromStdString(scriptable.scriptName));
				openCodeEditorButton->setEnabled(true);
				if (scriptable.scriptName != currentScript) {
					updateEditableScriptVariables(scriptable.scriptName);
					createEditableScriptVariablesWidget(scriptable.scriptName);
				}
				updateEditableScriptVariablesWidget(const_cast<Scriptable&>(scriptable));
				if (!m_currentScriptWatcher.files().empty()) {
					m_currentScriptWatcher.removePaths(m_currentScriptWatcher.files());
				}
				if (m_scriptToPath.find(scriptable.scriptName) != m_scriptToPath.end()) {
					m_currentScriptWatcher.addPath(QString::fromStdString(m_scriptToPath[scriptable.scriptName]));
				}
			}
			else {
				scriptNameWidget->comboBox->addItem(QString::fromStdString(scriptable.scriptName));
				scriptNameWidget->comboBox->setCurrentText(QString::fromStdString(scriptable.scriptName));
				openCodeEditorButton->setEnabled(false);
				editableScriptVariablesWidget->hide();
				if (!m_currentScriptWatcher.files().empty()) {
					m_currentScriptWatcher.removePaths(m_currentScriptWatcher.files());
				}
			}
		}
	}
	else {
		{
			const QSignalBlocker signalBlocker(scriptNameWidget->comboBox);
			scriptNameWidget->comboBox->setCurrentText(QString::fromStdString(m_globalInfo.localization.getString("component_scriptable_no_script_selected")));
		}
		openCodeEditorButton->setEnabled(false);
		editableScriptVariablesWidget->hide();
		if (!m_currentScriptWatcher.files().empty()) {
			m_currentScriptWatcher.removePaths(m_currentScriptWatcher.files());
		}
	}
}

void ScriptableComponentWidget::updateComponents(const std::vector<EntityID>& entityIDs, std::vector<Scriptable>& scriptables) {
	std::vector<Component*> componentPointers;
	for (size_t i = 0; i < scriptables.size(); i++) {
		componentPointers.push_back(&scriptables[i]);
	}

	m_globalInfo.actionUndoStack->push(new ChangeEntitiesComponentCommand(m_globalInfo, entityIDs, "Scriptable", componentPointers));
}

std::vector<std::string> ScriptableComponentWidget::getScriptEntries() {
	m_scriptToPath.clear();
	std::vector<std::string> scriptEntries;
	scriptEntries.push_back(m_globalInfo.localization.getString("component_scriptable_no_script_selected"));
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

						std::string scriptNoCase = "";
						for (char c : scriptName) {
							scriptNoCase += std::toupper(static_cast<unsigned char>(c));
						}
						m_scriptsNoCase.insert(scriptNoCase);
					}
				}
			}
		}
		m_scriptsDirectoryWatcher.addPath(QString::fromStdString(m_globalInfo.projectDirectory) + "/scripts/");
	}
	scriptEntries.push_back(m_globalInfo.localization.getString("component_scriptable_new_script"));

	return scriptEntries;
}

std::pair<std::string, std::pair<std::string, EditableScriptVariableValue>> ScriptableComponentWidget::parseVariableLineTokens(const std::vector<std::string>& tokens, bool usingNamespaceStd, bool usingNamespaceNtshEngnMath) {
	EditableScriptVariableValue editableScriptVariableValue;
	
	size_t tokenCount = tokens.size();
	if (tokenCount < 2) {
		return { "" , { "Unknown", editableScriptVariableValue } };
	}

	std::string type = tokens[0];
	std::string name = tokens[1];
	if (type == "bool") {
		if (tokenCount >= 4) {
			if (tokens[3] == "true") {
				editableScriptVariableValue = true;
			}
			else if (tokens[3] == "false") {
				editableScriptVariableValue = false;
			}
		}
		else {
			editableScriptVariableValue = false;
		}

		return { name, { "Boolean", editableScriptVariableValue } };
	}
	else if (type == "int8_t") {
		if (tokenCount >= 4) {
			editableScriptVariableValue = std::atoi(tokens[3].c_str());
		}
		else {
			editableScriptVariableValue = 0;
		}

		return { name, { "Int8", editableScriptVariableValue } };
	}
	else if (type == "int16_t") {
		if (tokenCount >= 4) {
			editableScriptVariableValue = std::atoi(tokens[3].c_str());
		}
		else {
			editableScriptVariableValue = 0;
		}

		return { name, { "Int16", editableScriptVariableValue } };
	}
	else if ((type == "int") || (type == "int32_t")) {
		if (tokenCount >= 4) {
			editableScriptVariableValue = std::atoi(tokens[3].c_str());
		}
		else {
			editableScriptVariableValue = 0;
		}

		return { name, { "Int32", editableScriptVariableValue } };
	}
	else if (type == "int64_t") {
		if (tokenCount >= 4) {
			editableScriptVariableValue = std::atoi(tokens[3].c_str());
		}
		else {
			editableScriptVariableValue = 0;
		}

		return { name, { "Int64", editableScriptVariableValue } };
	}
	else if (type == "uint8_t") {
		if (tokenCount >= 4) {
			editableScriptVariableValue = std::atoi(tokens[3].c_str());
		}
		else {
			editableScriptVariableValue = 0;
		}

		return { name, { "Uint8", editableScriptVariableValue } };
	}
	else if (type == "uint16_t") {
		if (tokenCount >= 4) {
			editableScriptVariableValue = std::atoi(tokens[3].c_str());
		}
		else {
			editableScriptVariableValue = 0;
		}

		return { name, { "Uint16", editableScriptVariableValue } };
	}
	else if (type == "uint32_t") {
		if (tokenCount >= 4) {
			editableScriptVariableValue = std::atoi(tokens[3].c_str());
		}
		else {
			editableScriptVariableValue = 0;
		}

		return { name, { "Uint32", editableScriptVariableValue } };
	}
	else if ((type == "uint64_t") || (type == "size_t")) {
		if (tokenCount >= 4) {
			editableScriptVariableValue = std::atoi(tokens[3].c_str());
		}
		else {
			editableScriptVariableValue = 0;
		}

		return { name, { "Uint64", editableScriptVariableValue } };
	}
	else if (type == "float") {
		if (tokenCount >= 4) {
			editableScriptVariableValue = static_cast<float>(std::atof(tokens[3].c_str()));
		}
		else {
			editableScriptVariableValue = 0.0f;
		}

		return { name, { "Float32", editableScriptVariableValue } };
	}
	else if (type == "double") {
		if (tokenCount >= 4) {
			editableScriptVariableValue = static_cast<float>(std::atof(tokens[3].c_str()));
		}
		else {
			editableScriptVariableValue = 0.0f;
		}

		return { name, { "Float64", editableScriptVariableValue } };
	}
	else if ((usingNamespaceStd && (type == "string")) || (!usingNamespaceStd && (type == "std::string"))) {
		if (tokenCount >= 4) {
			std::string stringToken = tokens[3];
			if (stringToken.front() == '\"') {
				stringToken = stringToken.substr(1);
			}
			if (stringToken.back() == '\"') {
				stringToken.pop_back();
			}
			editableScriptVariableValue = stringToken;
		}
		else {
			editableScriptVariableValue = std::string("");
		}

		return { name, { "String", editableScriptVariableValue } };
	}
	else if ((usingNamespaceNtshEngnMath && (type == "vec2")) || (!usingNamespaceNtshEngnMath && (type == "Math::vec2"))) {
		if (tokenCount >= 4) {
			nml::vec2 value = nml::vec2(0.0f, 0.0f);

			std::string fullVectorString = tokens[3];
			for (size_t i = 4; i < tokens.size(); i++) {
				fullVectorString += tokens[i];
			}
			std::string valueString = fullVectorString;
			size_t openParenthesisPos = valueString.find('(');
			if (openParenthesisPos != std::string::npos) {
				valueString = valueString.substr(openParenthesisPos + 1);
			}
			size_t closedParenthesisPos = valueString.find(')');
			if (closedParenthesisPos != std::string::npos) {
				valueString = valueString.substr(0, closedParenthesisPos);
			}
			size_t commaPos = valueString.find(',');
			if (commaPos == std::string::npos) {
				// One value for the vector
				float valueFloat = static_cast<float>(std::atof(valueString.c_str()));
				value = nml::vec2(valueFloat, valueFloat);
			}
			else {
				// Multiple values for the vector
				std::string valueXString = valueString.substr(0, commaPos);
				std::string valueYString = valueString.substr(commaPos + 1);
				value.x = static_cast<float>(std::atof(valueXString.c_str()));
				value.y = static_cast<float>(std::atof(valueYString.c_str()));
			}

			editableScriptVariableValue = value;
		}
		else {
			editableScriptVariableValue = nml::vec2(0.0f, 0.0f);
		}

		return { name, { "Vector2", editableScriptVariableValue } };
	}
	else if ((usingNamespaceNtshEngnMath && (type == "vec3")) || (!usingNamespaceNtshEngnMath && (type == "Math::vec3"))) {
		if (tokenCount >= 4) {
			nml::vec3 value = nml::vec3(0.0f, 0.0f, 0.0f);

			std::string fullVectorString = tokens[3];
			for (size_t i = 4; i < tokens.size(); i++) {
				fullVectorString += tokens[i];
			}
			std::string valueString = fullVectorString;
			size_t openParenthesisPos = valueString.find('(');
			if (openParenthesisPos != std::string::npos) {
				valueString = valueString.substr(openParenthesisPos + 1);
			}
			size_t closedParenthesisPos = valueString.find(')');
			if (closedParenthesisPos != std::string::npos) {
				valueString = valueString.substr(0, closedParenthesisPos);
			}
			size_t commaPos = valueString.find(',');
			if (commaPos == std::string::npos) {
				// One value for the vector
				float valueFloat = static_cast<float>(std::atof(valueString.c_str()));
				value = nml::vec3(valueFloat, valueFloat, valueFloat);
			}
			else {
				// Multiple values for the vector
				std::string valueXString = valueString.substr(0, commaPos);
				value.x = static_cast<float>(std::atof(valueXString.c_str()));

				valueString = valueString.substr(commaPos + 1);
				commaPos = valueString.find(',');
				if (commaPos != std::string::npos) {
					std::string valueYString = valueString.substr(0, commaPos);
					std::string valueZString = valueString.substr(commaPos + 1);
					value.y = static_cast<float>(std::atof(valueYString.c_str()));
					value.z = static_cast<float>(std::atof(valueZString.c_str()));
				}
			}

			editableScriptVariableValue = value;
		}
		else {
			editableScriptVariableValue = nml::vec3(0.0f, 0.0f, 0.0f);
		}

		return { name, { "Vector3", editableScriptVariableValue } };
	}
	else if ((usingNamespaceNtshEngnMath && (type == "vec4")) || (!usingNamespaceNtshEngnMath && (type == "Math::vec4"))) {
		if (tokenCount > 4) {
			nml::vec4 value = nml::vec4(0.0f, 0.0f, 0.0f, 0.0f);

			std::string fullVectorString = tokens[3];
			for (size_t i = 4; i < tokens.size(); i++) {
				fullVectorString += tokens[i];
			}
			std::string valueString = fullVectorString;
			size_t openParenthesisPos = valueString.find('(');
			if (openParenthesisPos != std::string::npos) {
				valueString = valueString.substr(openParenthesisPos + 1);
			}
			size_t closedParenthesisPos = valueString.find(')');
			if (closedParenthesisPos != std::string::npos) {
				valueString = valueString.substr(0, closedParenthesisPos);
			}
			size_t commaPos = valueString.find(',');
			if (commaPos == std::string::npos) {
				// One value for the vector
				float valueFloat = static_cast<float>(std::atof(valueString.c_str()));
				value = nml::vec4(valueFloat, valueFloat, valueFloat, valueFloat);
			}
			else {
				// Multiple values for the vector
				std::string valueXString = valueString.substr(0, commaPos);
				value.x = static_cast<float>(std::atof(valueXString.c_str()));

				valueString = valueString.substr(commaPos + 1);
				commaPos = valueString.find(',');
				if (commaPos != std::string::npos) {
					std::string valueYString = valueString.substr(0, commaPos);
					value.y = static_cast<float>(std::atof(valueYString.c_str()));

					valueString = valueString.substr(commaPos + 1);
					commaPos = valueString.find(',');
					if (commaPos != std::string::npos) {
						std::string valueZString = valueString.substr(0, commaPos);
						std::string valueWString = valueString.substr(commaPos + 1);
						value.z = static_cast<float>(std::atof(valueZString.c_str()));
						value.w = static_cast<float>(std::atof(valueWString.c_str()));
					}
				}
			}

			editableScriptVariableValue = value;
		}
		else {
			editableScriptVariableValue = nml::vec4(0.0f, 0.0f, 0.0f, 0.0f);
		}

		return { name, { "Vector4", editableScriptVariableValue } };
	}
	else if ((usingNamespaceNtshEngnMath && (type == "quat")) || (!usingNamespaceNtshEngnMath && (type == "Math::quat"))) {
		if (tokenCount > 4) {
			nml::vec4 value = nml::vec4(0.0f, 0.0f, 0.0f, 0.0f);

			std::string fullVectorString = tokens[3];
			for (size_t i = 4; i < tokens.size(); i++) {
				fullVectorString += tokens[i];
			}
			std::string valueString = fullVectorString;
			size_t openParenthesisPos = valueString.find('(');
			if (openParenthesisPos != std::string::npos) {
				valueString = valueString.substr(openParenthesisPos + 1);
			}
			size_t closedParenthesisPos = valueString.find('(');
			if (closedParenthesisPos != std::string::npos) {
				valueString = valueString.substr(0, closedParenthesisPos);
			}
			size_t commaPos = valueString.find(',');
			if (commaPos != std::string::npos) {
				// Multiple values for the quaternion
				std::string valueAString = valueString.substr(0, commaPos);
				value.x = static_cast<float>(std::atof(valueAString.c_str()));

				valueString = valueString.substr(commaPos + 1);
				commaPos = valueString.find(',');
				if (commaPos != std::string::npos) {
					std::string valueBString = valueString.substr(0, commaPos);
					value.y = static_cast<float>(std::atof(valueBString.c_str()));

					valueString = valueString.substr(commaPos + 1);
					commaPos = valueString.find(',');
					if (commaPos != std::string::npos) {
						std::string valueCString = valueString.substr(0, commaPos);
						std::string valueDString = valueString.substr(commaPos + 1);
						value.z = static_cast<float>(std::atof(valueCString.c_str()));
						value.w = static_cast<float>(std::atof(valueDString.c_str()));
					}
				}
			}

			editableScriptVariableValue = value;
		}
		else {
			editableScriptVariableValue = nml::vec4(0.0f, 0.0f, 0.0f, 0.0f);
		}

		return { name, { "Quaternion", editableScriptVariableValue } };
	}

	return { name, { type, editableScriptVariableValue } };
}

void ScriptableComponentWidget::updateEditableScriptVariables(const std::string& scriptName) {
	if (m_editableScriptVariables.find(scriptName) != m_editableScriptVariables.end()) {
		m_editableScriptVariables[scriptName].clear();
	}

	const std::string editableScriptVariableDefine = "NTSHENGN_EDITABLE_VARIABLE";

	if (m_scriptToPath.find(scriptName) != m_scriptToPath.end()) {
		std::string scriptPath = m_scriptToPath[scriptName];

		std::fstream scriptFile(scriptPath, std::ios::in);
		if (scriptFile.is_open()) {
			std::string scriptContent((std::istreambuf_iterator<char>(scriptFile)), std::istreambuf_iterator<char>());

			bool usingNamespaceStd = scriptContent.find("using namespace std;") != std::string::npos;
			bool usingNamespaceNtshEngnMath = scriptContent.find("using namespace NtshEngn::Math;") != std::string::npos;
			std::string currentParsing = scriptContent;
			size_t editableScriptVariableIndex;
			while ((editableScriptVariableIndex = currentParsing.find(editableScriptVariableDefine)) != std::string::npos) {
				currentParsing = currentParsing.substr(editableScriptVariableIndex + editableScriptVariableDefine.length() + 1);
				std::string variableLine = currentParsing.substr(0, currentParsing.find(';'));
				std::vector<std::string> variableLineTokens;
				size_t spacePosition = 0;
				while ((spacePosition = variableLine.find(' ')) != std::string::npos) {
					std::string variableLineToken = variableLine.substr(0, spacePosition);
					if (!variableLineToken.empty()) {
						variableLineTokens.push_back(variableLineToken);
					}
					variableLine.erase(0, spacePosition + 1);
				}
				if (!variableLine.empty()) {
					variableLineTokens.push_back(variableLine);
				}
				std::pair<std::string, std::pair<std::string, EditableScriptVariableValue>> newEditableScriptVariable = parseVariableLineTokens(variableLineTokens, usingNamespaceStd, usingNamespaceNtshEngnMath);
				m_editableScriptVariables[scriptName][newEditableScriptVariable.first] = newEditableScriptVariable.second;
			}
		}
	}
}

void ScriptableComponentWidget::createEditableScriptVariablesWidget(const std::string& scriptName) {
	while (QLayoutItem* item = editableScriptVariablesWidget->layout()->takeAt(0)) {
		delete item->widget();
		delete item;
	}
	m_widgetToEditableScriptVariableName.clear();
	editableScriptVariablesWidget->layout()->addWidget(new QLabel(QString::fromStdString(m_globalInfo.localization.getString("component_scriptable_editable_variables"))));

	if (m_scriptToPath.find(scriptName) != m_scriptToPath.end()) {
		for (const auto& editableScriptVariable : m_editableScriptVariables[scriptName]) {
			std::string name = editableScriptVariable.first;
			std::string type = editableScriptVariable.second.first;
			if (type != "Unknown") {
				if (type == "Boolean") {
					BooleanWidget* widget = new BooleanWidget(m_globalInfo, name);
					widget->setValue(std::get<bool>(editableScriptVariable.second.second));

					m_widgetToEditableScriptVariableName[widget] = name;
					m_editableScriptVariableNameToWidget[name] = widget;

					connect(widget, &BooleanWidget::stateChanged, this, &ScriptableComponentWidget::onEditableScriptVariableChanged);

					editableScriptVariablesWidget->layout()->addWidget(widget);
				}
				else if (type == "Int8") {
					IntegerWidget* widget = new IntegerWidget(m_globalInfo, name);
					widget->setValue(std::get<int>(editableScriptVariable.second.second));
					widget->setMin(std::numeric_limits<int8_t>::min());
					widget->setMax(std::numeric_limits<int8_t>::max());

					m_widgetToEditableScriptVariableName[widget] = name;
					m_editableScriptVariableNameToWidget[name] = widget;

					connect(widget, &IntegerWidget::valueChanged, this, &ScriptableComponentWidget::onEditableScriptVariableChanged);

					editableScriptVariablesWidget->layout()->addWidget(widget);
				}
				else if (type == "Int16") {
					IntegerWidget* widget = new IntegerWidget(m_globalInfo, name);
					widget->setValue(std::get<int>(editableScriptVariable.second.second));
					widget->setMin(std::numeric_limits<int16_t>::min());
					widget->setMax(std::numeric_limits<int16_t>::max());

					m_widgetToEditableScriptVariableName[widget] = name;
					m_editableScriptVariableNameToWidget[name] = widget;

					connect(widget, &IntegerWidget::valueChanged, this, &ScriptableComponentWidget::onEditableScriptVariableChanged);

					editableScriptVariablesWidget->layout()->addWidget(widget);
				}
				else if (type == "Int32") {
					IntegerWidget* widget = new IntegerWidget(m_globalInfo, name);
					widget->setValue(std::get<int>(editableScriptVariable.second.second));

					m_widgetToEditableScriptVariableName[widget] = name;
					m_editableScriptVariableNameToWidget[name] = widget;

					connect(widget, &IntegerWidget::valueChanged, this, &ScriptableComponentWidget::onEditableScriptVariableChanged);

					editableScriptVariablesWidget->layout()->addWidget(widget);
				}
				else if (type == "Int64") {
					IntegerWidget* widget = new IntegerWidget(m_globalInfo, name);
					widget->setValue(std::get<int>(editableScriptVariable.second.second));

					m_widgetToEditableScriptVariableName[widget] = name;
					m_editableScriptVariableNameToWidget[name] = widget;

					connect(widget, &IntegerWidget::valueChanged, this, &ScriptableComponentWidget::onEditableScriptVariableChanged);

					editableScriptVariablesWidget->layout()->addWidget(widget);
				}
				else if (type == "Uint8") {
					IntegerWidget* widget = new IntegerWidget(m_globalInfo, name);
					widget->setValue(std::get<int>(editableScriptVariable.second.second));
					widget->setMin(std::numeric_limits<uint8_t>::min());
					widget->setMax(std::numeric_limits<uint8_t>::max());

					m_widgetToEditableScriptVariableName[widget] = name;
					m_editableScriptVariableNameToWidget[name] = widget;

					connect(widget, &IntegerWidget::valueChanged, this, &ScriptableComponentWidget::onEditableScriptVariableChanged);

					editableScriptVariablesWidget->layout()->addWidget(widget);
				}
				else if (type == "Uint16") {
					IntegerWidget* widget = new IntegerWidget(m_globalInfo, name);
					widget->setValue(std::get<int>(editableScriptVariable.second.second));
					widget->setMin(std::numeric_limits<uint16_t>::min());
					widget->setMax(std::numeric_limits<uint16_t>::max());

					m_widgetToEditableScriptVariableName[widget] = name;
					m_editableScriptVariableNameToWidget[name] = widget;

					connect(widget, &IntegerWidget::valueChanged, this, &ScriptableComponentWidget::onEditableScriptVariableChanged);

					editableScriptVariablesWidget->layout()->addWidget(widget);
				}
				else if (type == "Uint32") {
					IntegerWidget* widget = new IntegerWidget(m_globalInfo, name);
					widget->setValue(std::get<int>(editableScriptVariable.second.second));

					m_widgetToEditableScriptVariableName[widget] = name;
					m_editableScriptVariableNameToWidget[name] = widget;

					connect(widget, &IntegerWidget::valueChanged, this, &ScriptableComponentWidget::onEditableScriptVariableChanged);

					editableScriptVariablesWidget->layout()->addWidget(widget);
				}
				else if (type == "Uint64") {
					IntegerWidget* widget = new IntegerWidget(m_globalInfo, name);
					widget->setValue(std::get<int>(editableScriptVariable.second.second));

					m_widgetToEditableScriptVariableName[widget] = name;
					m_editableScriptVariableNameToWidget[name] = widget;

					connect(widget, &IntegerWidget::valueChanged, this, &ScriptableComponentWidget::onEditableScriptVariableChanged);

					editableScriptVariablesWidget->layout()->addWidget(widget);
				}
				else if (type == "Float32") {
					ScalarWidget* widget = new ScalarWidget(m_globalInfo, name);
					widget->setValue(std::get<float>(editableScriptVariable.second.second));

					m_widgetToEditableScriptVariableName[widget] = name;
					m_editableScriptVariableNameToWidget[name] = widget;

					connect(widget, &ScalarWidget::valueChanged, this, &ScriptableComponentWidget::onEditableScriptVariableChanged);

					editableScriptVariablesWidget->layout()->addWidget(widget);
				}
				else if (type == "Float64") {
					ScalarWidget* widget = new ScalarWidget(m_globalInfo, name);
					widget->setValue(std::get<float>(editableScriptVariable.second.second));

					m_widgetToEditableScriptVariableName[widget] = name;
					m_editableScriptVariableNameToWidget[name] = widget;

					connect(widget, &ScalarWidget::valueChanged, this, &ScriptableComponentWidget::onEditableScriptVariableChanged);

					editableScriptVariablesWidget->layout()->addWidget(widget);
				}
				else if (type == "String") {
					StringWidget* widget = new StringWidget(m_globalInfo, name);
					widget->setText(std::get<std::string>(editableScriptVariable.second.second));

					m_widgetToEditableScriptVariableName[widget] = name;
					m_editableScriptVariableNameToWidget[name] = widget;

					connect(widget, &StringWidget::valueChanged, this, &ScriptableComponentWidget::onEditableScriptVariableChanged);

					editableScriptVariablesWidget->layout()->addWidget(widget);
				}
				else if (type == "Vector2") {
					Vector2Widget* widget = new Vector2Widget(m_globalInfo, name);
					widget->setValue(std::get<nml::vec2>(editableScriptVariable.second.second));

					m_widgetToEditableScriptVariableName[widget] = name;
					m_editableScriptVariableNameToWidget[name] = widget;

					connect(widget, &Vector2Widget::valueChanged, this, &ScriptableComponentWidget::onEditableScriptVariableChanged);

					editableScriptVariablesWidget->layout()->addWidget(widget);
				}
				else if (type == "Vector3") {
					Vector3Widget* widget = new Vector3Widget(m_globalInfo, name);
					widget->setValue(std::get<nml::vec3>(editableScriptVariable.second.second));

					m_widgetToEditableScriptVariableName[widget] = name;
					m_editableScriptVariableNameToWidget[name] = widget;

					connect(widget, &Vector3Widget::valueChanged, this, &ScriptableComponentWidget::onEditableScriptVariableChanged);

					editableScriptVariablesWidget->layout()->addWidget(widget);
				}
				else if (type == "Vector4") {
					Vector4Widget* widget = new Vector4Widget(m_globalInfo, name);
					widget->setValue(std::get<nml::vec4>(editableScriptVariable.second.second));

					m_widgetToEditableScriptVariableName[widget] = name;
					m_editableScriptVariableNameToWidget[name] = widget;

					connect(widget, &Vector4Widget::valueChanged, this, &ScriptableComponentWidget::onEditableScriptVariableChanged);

					editableScriptVariablesWidget->layout()->addWidget(widget);
				}
				else if (type == "Quaternion") {
					QuaternionWidget* widget = new QuaternionWidget(m_globalInfo, name);
					nml::vec4 value = std::get<nml::vec4>(editableScriptVariable.second.second);
					widget->setValue(nml::quat(value.x, value.y, value.z, value.w));

					m_widgetToEditableScriptVariableName[widget] = name;
					m_editableScriptVariableNameToWidget[name] = widget;

					connect(widget, &QuaternionWidget::valueChanged, this, &ScriptableComponentWidget::onEditableScriptVariableChanged);

					editableScriptVariablesWidget->layout()->addWidget(widget);
				}
				else {
					m_globalInfo.logger.addLog(LogLevel::Warning, m_globalInfo.localization.getString("log_script_unsupported_editable_variable_type", { scriptName, name, type }));
				}
			}
		}
	}

	if (editableScriptVariablesWidget->layout()->count() == 1) {
		editableScriptVariablesWidget->hide();
	}
	else {
		editableScriptVariablesWidget->show();
	}
}

void ScriptableComponentWidget::updateEditableScriptVariablesWidget(Scriptable& scriptable) {
	if (m_scriptToPath.find(scriptable.scriptName) != m_scriptToPath.end()) {
		for (const auto& editableScriptVariable : m_editableScriptVariables[scriptable.scriptName]) {
			std::string name = editableScriptVariable.first;
			std::string type = editableScriptVariable.second.first;
			if (m_editableScriptVariableNameToWidget.find(name) != m_editableScriptVariableNameToWidget.end()) {
				EditableScriptVariableValue editableScriptVariableValue;
				if (scriptable.editableVariables.find(name) != scriptable.editableVariables.end()) {
					editableScriptVariableValue = scriptable.editableVariables[name];
				}
				else {
					editableScriptVariableValue = editableScriptVariable.second.second;
				}

				QWidget* baseWidget = m_editableScriptVariableNameToWidget[name];
				if (type == "Boolean") {
					BooleanWidget* widget = static_cast<BooleanWidget*>(baseWidget);
					widget->setValue(std::get<bool>(editableScriptVariableValue));
				}
				else if ((type == "Int8") ||
					(type == "Int16") ||
					(type == "Int32") ||
					(type == "Int64") ||
					(type == "Uint8") ||
					(type == "Uint16") ||
					(type == "Uint32") ||
					(type == "Uint64")) {
					IntegerWidget* widget = static_cast<IntegerWidget*>(baseWidget);
					widget->setValue(std::get<int>(editableScriptVariableValue));
				}
				else if ((type == "Float32") ||
					(type == "Float64")) {
					ScalarWidget* widget = static_cast<ScalarWidget*>(baseWidget);
					widget->setValue(std::get<float>(editableScriptVariableValue));
				}
				else if (type == "String") {
					StringWidget* widget = static_cast<StringWidget*>(baseWidget);
					widget->setText(std::get<std::string>(editableScriptVariableValue));
				}
				else if (type == "Vector2") {
					Vector2Widget* widget = static_cast<Vector2Widget*>(baseWidget);
					widget->setValue(std::get<nml::vec2>(editableScriptVariableValue));
				}
				else if (type == "Vector3") {
					Vector3Widget* widget = static_cast<Vector3Widget*>(baseWidget);
					widget->setValue(std::get<nml::vec3>(editableScriptVariableValue));
				}
				else if (type == "Vector4") {
					Vector4Widget* widget = static_cast<Vector4Widget*>(baseWidget);
					widget->setValue(std::get<nml::vec4>(editableScriptVariableValue));
				}
				else if (type == "Quaternion") {
					QuaternionWidget* widget = static_cast<QuaternionWidget*>(baseWidget);
					nml::vec4 value = std::get<nml::vec4>(editableScriptVariableValue);
					widget->setValue(nml::quat(value.x, value.y, value.z, value.w));
				}
			}
		}
	}
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
	QObject* senderWidget = sender();

	std::string newScriptName = "";
	if (senderWidget == scriptNameWidget) {
		if (element == m_globalInfo.localization.getString("component_scriptable_new_script")) {
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

					const std::regex validFilenameRegex(R"(^[a-zA-Z0-9._ -]+$)");
					if (std::regex_search(scriptName, validFilenameRegex)) {
						std::string scriptNoCase = "";
						for (char c : scriptName) {
							scriptNoCase += std::toupper(static_cast<unsigned char>(c));
						}

						std::string finalScriptName = scriptName;
						uint32_t scriptNameIndex = 0;
						if (m_scriptsNoCase.find(scriptNoCase) != m_scriptsNoCase.end()) {
							while (m_scriptsNoCase.find(scriptNoCase + "_" + std::to_string(scriptNameIndex)) != m_scriptsNoCase.end()) {
								scriptNameIndex++;
							}
							finalScriptName = scriptName + "_" + std::to_string(scriptNameIndex);
							m_globalInfo.logger.addLog(LogLevel::Warning, m_globalInfo.localization.getString("log_script_name_already_taken", { scriptName, finalScriptName }));
						}

						std::fstream newScriptFile(m_globalInfo.projectDirectory + "/scripts/" + finalScriptName + ".h", std::ios::out | std::ios::trunc);
						newScriptFile << "#pragma once\n#include \"../Common/script/ntshengn_script.h\"\n\nusing namespace NtshEngn;\nstruct " << finalScriptName << " : public Script {\n\tNTSHENGN_SCRIPT(" << finalScriptName << ");\n\n\tvoid init() {\n\n\t}\n\n\tvoid update(float dt) {\n\t\tNTSHENGN_UNUSED(dt);\n\t}\n\n\tvoid destroy() {\n\n\t}\n};";

						newScriptName = finalScriptName;
					}
					else {
						m_globalInfo.logger.addLog(LogLevel::Warning, m_globalInfo.localization.getString("log_script_name_not_valid", { scriptName }));
					}
				}
			}
		}
	}

	std::vector<EntityID> entityIDs;
	std::vector<Scriptable> newScriptables;

	std::set<EntityID> selectedEntityIDs = m_globalInfo.otherSelectedEntityIDs;
	selectedEntityIDs.insert(m_globalInfo.currentEntityID);
	for (EntityID selectedEntityID : selectedEntityIDs) {
		if (m_globalInfo.entities[selectedEntityID].scriptable) {
			Scriptable newScriptable = m_globalInfo.entities[selectedEntityID].scriptable.value();

			if (senderWidget == scriptNameWidget) {
				if (element == m_globalInfo.localization.getString("component_scriptable_no_script_selected")) {
					newScriptable.scriptName = "";
				}
				else if (element == m_globalInfo.localization.getString("component_scriptable_new_script")) {
					newScriptable.scriptName = newScriptName;
				}
				else {
					newScriptable.scriptName = element;
				}
			}

			entityIDs.push_back(selectedEntityID);
			newScriptables.push_back(newScriptable);

			if (selectedEntityID == m_globalInfo.currentEntityID) {
				updateEditableScriptVariables(newScriptable.scriptName);
				createEditableScriptVariablesWidget(newScriptable.scriptName);
				updateEditableScriptVariablesWidget(newScriptable);
			}
		}
	}

	updateComponents(entityIDs, newScriptables);
}

void ScriptableComponentWidget::onOpenCodeEditorButtonClicked() {
	std::string codeEditorCommand = m_globalInfo.editorParameters.code.codeEditorCommand;

	if (codeEditorCommand.empty()) {
		m_globalInfo.logger.addLog(LogLevel::Warning, m_globalInfo.localization.getString("log_code_editor_none"));

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

void ScriptableComponentWidget::onCurrentScriptChanged(const QString& path) {
	if (std::filesystem::exists(path.toStdString())) {
		if ((m_globalInfo.currentEntityID != NO_ENTITY) && m_globalInfo.entities[m_globalInfo.currentEntityID].scriptable.has_value()) {
			updateEditableScriptVariables(m_globalInfo.entities[m_globalInfo.currentEntityID].scriptable->scriptName);
			createEditableScriptVariablesWidget(m_globalInfo.entities[m_globalInfo.currentEntityID].scriptable->scriptName);
			updateEditableScriptVariablesWidget(m_globalInfo.entities[m_globalInfo.currentEntityID].scriptable.value());
		}
	}
}

void ScriptableComponentWidget::onEditableScriptVariableChanged() {
	QObject* senderWidget = sender();

	std::vector<EntityID> entityIDs;
	std::vector<Scriptable> newScriptables;

	std::set<EntityID> selectedEntityIDs = m_globalInfo.otherSelectedEntityIDs;
	selectedEntityIDs.insert(m_globalInfo.currentEntityID);
	for (EntityID selectedEntityID : selectedEntityIDs) {
		if (m_globalInfo.entities[selectedEntityID].scriptable) {
			if (m_globalInfo.entities[selectedEntityID].scriptable->scriptName != m_globalInfo.entities[m_globalInfo.currentEntityID].scriptable->scriptName) { // Do not modify editable variables if it is not the same script as the current entity
				continue;
			}

			Scriptable newScriptable = m_globalInfo.entities[selectedEntityID].scriptable.value();

			for (const auto& widgetToEditableScriptVariableName : m_widgetToEditableScriptVariableName) {
				if (senderWidget == widgetToEditableScriptVariableName.first) {
					auto& editableScriptVariables = m_editableScriptVariables[newScriptable.scriptName];
					std::string name = widgetToEditableScriptVariableName.second;
					if (editableScriptVariables.find(name) != editableScriptVariables.end()) {
						auto& editableScriptVariable = editableScriptVariables[name];
						std::string type = editableScriptVariable.first;
						if (type != "Unknown") {
							if (type == "Boolean") {
								BooleanWidget* widget = static_cast<BooleanWidget*>(senderWidget);
								bool value = widget->getValue();
								if (value != std::get<bool>(editableScriptVariable.second)) {
									newScriptable.editableVariables[name] = value;
								}
								else {
									newScriptable.editableVariables.erase(name);
									widget->setValue(std::get<bool>(editableScriptVariable.second));
								}
							}
							else if ((type == "Int8") ||
								(type == "Int16") ||
								(type == "Int32") ||
								(type == "Int64") ||
								(type == "Uint8") ||
								(type == "Uint16") ||
								(type == "Uint32") ||
								(type == "Uint64")) {
								IntegerWidget* widget = static_cast<IntegerWidget*>(senderWidget);
								int value = widget->getValue();
								if (value != std::get<int>(editableScriptVariable.second)) {
									newScriptable.editableVariables[name] = value;
								}
								else {
									newScriptable.editableVariables.erase(name);
									widget->setValue(std::get<int>(editableScriptVariable.second));
								}
							}
							else if ((type == "Float32") ||
								(type == "Float64")) {
								ScalarWidget* widget = static_cast<ScalarWidget*>(senderWidget);
								float value = widget->getValue();
								if (value != std::get<float>(editableScriptVariable.second)) {
									newScriptable.editableVariables[name] = value;
								}
								else {
									newScriptable.editableVariables.erase(name);
									widget->setValue(std::get<float>(editableScriptVariable.second));
								}
							}
							else if (type == "String") {
								StringWidget* widget = static_cast<StringWidget*>(senderWidget);
								std::string value = widget->getText();
								if (value != std::get<std::string>(editableScriptVariable.second)) {
									newScriptable.editableVariables[name] = value;
								}
								else {
									newScriptable.editableVariables.erase(name);
									widget->setText(std::get<std::string>(editableScriptVariable.second));
								}
							}
							else if (type == "Vector2") {
								Vector2Widget* widget = static_cast<Vector2Widget*>(senderWidget);
								nml::vec2 value = widget->getValue();
								if (value != std::get<nml::vec2>(editableScriptVariable.second)) {
									newScriptable.editableVariables[name] = value;
								}
								else {
									newScriptable.editableVariables.erase(name);
									widget->setValue(std::get<nml::vec2>(editableScriptVariable.second));
								}
							}
							else if (type == "Vector3") {
								Vector3Widget* widget = static_cast<Vector3Widget*>(senderWidget);
								nml::vec3 value = widget->getValue();
								if (value != std::get<nml::vec3>(editableScriptVariable.second)) {
									newScriptable.editableVariables[name] = value;
								}
								else {
									newScriptable.editableVariables.erase(name);
									widget->setValue(std::get<nml::vec3>(editableScriptVariable.second));
								}
							}
							else if (type == "Vector4") {
								Vector4Widget* widget = static_cast<Vector4Widget*>(senderWidget);
								nml::vec4 value = widget->getValue();
								if (value != std::get<nml::vec4>(editableScriptVariable.second)) {
									newScriptable.editableVariables[name] = value;
								}
								else {
									newScriptable.editableVariables.erase(name);
									widget->setValue(std::get<nml::vec4>(editableScriptVariable.second));
								}
							}
							else if (type == "Quaternion") {
								QuaternionWidget* widget = static_cast<QuaternionWidget*>(senderWidget);
								nml::quat valueQuat = widget->getValue();
								nml::vec4 value = nml::vec4(valueQuat.a, valueQuat.b, valueQuat.c, valueQuat.d);
								if (value != std::get<nml::vec4>(editableScriptVariable.second)) {
									newScriptable.editableVariables[name] = value;
								}
								else {
									newScriptable.editableVariables.erase(name);
									nml::vec4 editableScriptVariableVec4 = std::get<nml::vec4>(editableScriptVariable.second);
									nml::quat editableScriptVariableQuat = nml::quat(editableScriptVariableVec4.x, editableScriptVariableVec4.y, editableScriptVariableVec4.z, editableScriptVariableVec4.w);
									widget->setValue(editableScriptVariableQuat);
								}
							}
						}
					}

					break;
				}
			}

			entityIDs.push_back(selectedEntityID);
			newScriptables.push_back(newScriptable);
		}
	}

	updateComponents(entityIDs, newScriptables);
}
