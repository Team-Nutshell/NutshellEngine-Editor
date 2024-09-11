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
#include <cstdlib>
#include <cstdint>

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
	editableVariablesWidget = new QWidget();
	editableVariablesWidget->setLayout(new QVBoxLayout());
	editableVariablesWidget->layout()->setContentsMargins(0, 0, 0, 0);
	editableVariablesWidget->layout()->addWidget(new QLabel("Editable Variables:"));
	layout()->addWidget(editableVariablesWidget);
	layout()->addWidget(new SeparatorLine(m_globalInfo));

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
					updateEditableVariablesWidget(const_cast<Scriptable&>(scriptable));
				}
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
				editableVariablesWidget->hide();
				if (!m_currentScriptWatcher.files().empty()) {
					m_currentScriptWatcher.removePaths(m_currentScriptWatcher.files());
				}
			}
		}
	}
	else {
		{
			const QSignalBlocker signalBlocker(scriptNameWidget->comboBox);
			scriptNameWidget->comboBox->setCurrentText("No script selected");
		}
		openCodeEditorButton->setEnabled(false);
		editableVariablesWidget->hide();
		if (!m_currentScriptWatcher.files().empty()) {
			m_currentScriptWatcher.removePaths(m_currentScriptWatcher.files());
		}
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

std::tuple<std::string, std::string, EditableScriptVariableValue> ScriptableComponentWidget::parseVariableLineTokens(const std::vector<std::string>& tokens, bool usingNamespaceStd, bool usingNamespaceNtshEngnMath) {
	EditableScriptVariableValue editableVariableValue;
	
	size_t tokenCount = tokens.size();
	if (tokenCount < 2) {
		return { "" , "Unknown", editableVariableValue };
	}

	std::string type = tokens[0];
	std::string name = tokens[1];
	if (type == "bool") {
		if (tokenCount >= 4) {
			if (tokens[3] == "true") {
				editableVariableValue = true;
			}
			else if (tokens[3] == "false") {
				editableVariableValue = false;
			}
		}
		else {
			editableVariableValue = false;
		}

		return { name, "Boolean", editableVariableValue };
	}
	else if (type == "int8_t") {
		if (tokenCount >= 4) {
			editableVariableValue = std::atoi(tokens[3].c_str());
		}
		else {
			editableVariableValue = 0;
		}

		return { name, "Int8", editableVariableValue };
	}
	else if (type == "int16_t") {
		if (tokenCount >= 4) {
			editableVariableValue = std::atoi(tokens[3].c_str());
		}
		else {
			editableVariableValue = 0;
		}

		return { name, "Int16", editableVariableValue };
	}
	else if ((type == "int") || (type == "int32_t")) {
		if (tokenCount >= 4) {
			editableVariableValue = std::atoi(tokens[3].c_str());
		}
		else {
			editableVariableValue = 0;
		}

		return { name, "Int32", editableVariableValue };
	}
	else if (type == "int64_t") {
		if (tokenCount >= 4) {
			editableVariableValue = std::atoi(tokens[3].c_str());
		}
		else {
			editableVariableValue = 0;
		}

		return { name, "Int64", editableVariableValue };
	}
	else if (type == "uint8_t") {
		if (tokenCount >= 4) {
			editableVariableValue = std::atoi(tokens[3].c_str());
		}
		else {
			editableVariableValue = 0;
		}

		return { name, "Uint8", editableVariableValue };
	}
	else if (type == "uint16_t") {
		if (tokenCount >= 4) {
			editableVariableValue = std::atoi(tokens[3].c_str());
		}
		else {
			editableVariableValue = 0;
		}

		return { name, "Uint16", editableVariableValue };
	}
	else if (type == "uint32_t") {
		if (tokenCount >= 4) {
			editableVariableValue = std::atoi(tokens[3].c_str());
		}
		else {
			editableVariableValue = 0;
		}

		return { name, "Uint32", editableVariableValue };
	}
	else if ((type == "uint64_t") || (type == "size_t")) {
		if (tokenCount >= 4) {
			editableVariableValue = std::atoi(tokens[3].c_str());
		}
		else {
			editableVariableValue = 0;
		}

		return { name, "Uint64", editableVariableValue };
	}
	else if (type == "float") {
		if (tokenCount >= 4) {
			editableVariableValue = std::atof(tokens[3].c_str());
		}
		else {
			editableVariableValue = 0.0;
		}

		return { name, "Float32", editableVariableValue };
	}
	else if (type == "double") {
		if (tokenCount >= 4) {
			editableVariableValue = std::atof(tokens[3].c_str());
		}
		else {
			editableVariableValue = 0.0;
		}

		return { name, "Float64", editableVariableValue };
	}
	else if ((usingNamespaceStd && (type == "string")) || (!usingNamespaceStd && (type == "std::string"))) {
		if (tokenCount >= 4) {
			editableVariableValue = tokens[3];
		}
		else {
			editableVariableValue = std::string("");
		}

		return { name, "String", editableVariableValue };
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

			editableVariableValue = value;
		}
		else {
			editableVariableValue = nml::vec2(0.0f, 0.0f);
		}

		return { name, "Vector2", editableVariableValue };
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

			editableVariableValue = value;
		}
		else {
			editableVariableValue = nml::vec3(0.0f, 0.0f, 0.0f);
		}

		return { name, "Vector3", editableVariableValue };
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

			editableVariableValue = value;
		}
		else {
			editableVariableValue = nml::vec4(0.0f, 0.0f, 0.0f, 0.0f);
		}

		return { name, "Vector4", editableVariableValue };
	}
	else if ((usingNamespaceNtshEngnMath && (type == "quat")) || (!usingNamespaceNtshEngnMath && (type == "Math::quat"))) {
		if (tokenCount > 4) {
			nml::quat value = nml::quat(0.0f, 0.0f, 0.0f, 0.0f);

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
				value.a = static_cast<float>(std::atof(valueAString.c_str()));

				valueString = valueString.substr(commaPos + 1);
				commaPos = valueString.find(',');
				if (commaPos != std::string::npos) {
					std::string valueBString = valueString.substr(0, commaPos);
					value.b = static_cast<float>(std::atof(valueBString.c_str()));

					valueString = valueString.substr(commaPos + 1);
					commaPos = valueString.find(',');
					if (commaPos != std::string::npos) {
						std::string valueCString = valueString.substr(0, commaPos);
						std::string valueDString = valueString.substr(commaPos + 1);
						value.c = static_cast<float>(std::atof(valueCString.c_str()));
						value.d = static_cast<float>(std::atof(valueDString.c_str()));
					}
				}
			}
		}
		else {
			editableVariableValue = nml::quat(0.0f, 0.0f, 0.0f, 0.0f);
		}

		return { name, "Quaternion", editableVariableValue };
	}

	return { name, type, editableVariableValue };
}

void ScriptableComponentWidget::updateEditableVariablesWidget(Scriptable& scriptable) {
	while (QLayoutItem* item = editableVariablesWidget->layout()->takeAt(0)) {
		delete item->widget();
		delete item;
	}
	m_editableVariables.clear();
	m_widgetToEditableVariableName.clear();
	editableVariablesWidget->layout()->addWidget(new QLabel("Editable Variables:"));

	const std::string editableVariableDefine = "NTSHENGN_EDITABLE_VARIABLE";

	if (m_scriptToPath.find(scriptable.scriptName) != m_scriptToPath.end()) {
		std::string scriptPath = m_scriptToPath[scriptable.scriptName];

		std::fstream scriptFile(scriptPath, std::ios::in);
		if (scriptFile.is_open()) {
			std::string scriptContent((std::istreambuf_iterator<char>(scriptFile)), std::istreambuf_iterator<char>());

			bool usingNamespaceStd = scriptContent.find("using namespace std;") != std::string::npos;
			bool usingNamespaceNtshEngnMath = scriptContent.find("using namespace NtshEngn::Math;") != std::string::npos;
			std::string currentParsing = scriptContent;
			size_t editableVariableIndex;
			while ((editableVariableIndex = currentParsing.find(editableVariableDefine)) != std::string::npos) {
				currentParsing = currentParsing.substr(editableVariableIndex + editableVariableDefine.length() + 1);
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
				m_editableVariables.push_back(parseVariableLineTokens(variableLineTokens, usingNamespaceStd, usingNamespaceNtshEngnMath));
			}

			for (const auto& editableVariable : m_editableVariables) {
				std::string name = std::get<0>(editableVariable);
				std::string type = std::get<1>(editableVariable);
				if (type != "Unknown") {
					if (type == "Boolean") {
						BooleanWidget* widget = new BooleanWidget(m_globalInfo, name);
						widget->setValue(std::get<bool>(std::get<2>(editableVariable)));
						if (scriptable.editableVariables.find(name) != scriptable.editableVariables.end()) {
							if (scriptable.editableVariables[name].first == type) {
								widget->setValue(std::get<bool>(scriptable.editableVariables[name].second));
							}
							else {
								scriptable.editableVariables.erase(name);
							}
						}

						m_widgetToEditableVariableName[widget] = name;

						connect(widget, &BooleanWidget::stateChanged, this, &ScriptableComponentWidget::onEditableVariableChanged);
						
						editableVariablesWidget->layout()->addWidget(widget);
					}
					else if (type == "Int8") {
						IntegerWidget* widget = new IntegerWidget(m_globalInfo, name);
						widget->setValue(std::get<int>(std::get<2>(editableVariable)));
						widget->setMin(std::numeric_limits<int8_t>::min());
						widget->setMax(std::numeric_limits<int8_t>::max());
						if (scriptable.editableVariables.find(name) != scriptable.editableVariables.end()) {
							if (scriptable.editableVariables[name].first == type) {
								widget->setValue(std::get<int>(scriptable.editableVariables[name].second));
							}
							else {
								scriptable.editableVariables.erase(name);
							}
						}

						m_widgetToEditableVariableName[widget] = name;

						connect(widget, &IntegerWidget::valueChanged, this, &ScriptableComponentWidget::onEditableVariableChanged);
						
						editableVariablesWidget->layout()->addWidget(widget);
					}
					else if (type == "Int16") {
						IntegerWidget* widget = new IntegerWidget(m_globalInfo, name);
						widget->setValue(std::get<int>(std::get<2>(editableVariable)));
						widget->setMin(std::numeric_limits<int16_t>::min());
						widget->setMax(std::numeric_limits<int16_t>::max());
						if (scriptable.editableVariables.find(name) != scriptable.editableVariables.end()) {
							if (scriptable.editableVariables[name].first == type) {
								widget->setValue(std::get<int>(scriptable.editableVariables[name].second));
							}
							else {
								scriptable.editableVariables.erase(name);
							}
						}
						
						m_widgetToEditableVariableName[widget] = name;
						
						connect(widget, &IntegerWidget::valueChanged, this, &ScriptableComponentWidget::onEditableVariableChanged);
						
						editableVariablesWidget->layout()->addWidget(widget);
					}
					else if (type == "Int32") {
						IntegerWidget* widget = new IntegerWidget(m_globalInfo, name);
						widget->setValue(std::get<int>(std::get<2>(editableVariable)));
						if (scriptable.editableVariables.find(name) != scriptable.editableVariables.end()) {
							if (scriptable.editableVariables[name].first == type) {
								widget->setValue(std::get<int>(scriptable.editableVariables[name].second));
							}
							else {
								scriptable.editableVariables.erase(name);
							}
						}
						
						m_widgetToEditableVariableName[widget] = name;
						
						connect(widget, &IntegerWidget::valueChanged, this, &ScriptableComponentWidget::onEditableVariableChanged);
						
						editableVariablesWidget->layout()->addWidget(widget);
					}
					else if (type == "Int64") {
						IntegerWidget* widget = new IntegerWidget(m_globalInfo, name);
						widget->setValue(std::get<int>(std::get<2>(editableVariable)));
						if (scriptable.editableVariables.find(name) != scriptable.editableVariables.end()) {
							if (scriptable.editableVariables[name].first == type) {
								widget->setValue(std::get<int>(scriptable.editableVariables[name].second));
							}
							else {
								scriptable.editableVariables.erase(name);
							}
						}
						
						m_widgetToEditableVariableName[widget] = name;
						
						connect(widget, &IntegerWidget::valueChanged, this, &ScriptableComponentWidget::onEditableVariableChanged);
						
						editableVariablesWidget->layout()->addWidget(widget);
					}
					else if (type == "Uint8") {
						IntegerWidget* widget = new IntegerWidget(m_globalInfo, name);
						widget->setValue(std::get<int>(std::get<2>(editableVariable)));
						widget->setMin(std::numeric_limits<uint8_t>::min());
						widget->setMax(std::numeric_limits<uint8_t>::max());
						if (scriptable.editableVariables.find(name) != scriptable.editableVariables.end()) {
							if (scriptable.editableVariables[name].first == type) {
								widget->setValue(std::get<int>(scriptable.editableVariables[name].second));
							}
							else {
								scriptable.editableVariables.erase(name);
							}
						}
						
						m_widgetToEditableVariableName[widget] = name;
						
						connect(widget, &IntegerWidget::valueChanged, this, &ScriptableComponentWidget::onEditableVariableChanged);
						
						editableVariablesWidget->layout()->addWidget(widget);
					}
					else if (type == "Uint16") {
						IntegerWidget* widget = new IntegerWidget(m_globalInfo, name);
						widget->setValue(std::get<int>(std::get<2>(editableVariable)));
						widget->setMin(std::numeric_limits<uint16_t>::min());
						widget->setMax(std::numeric_limits<uint16_t>::max());
						if (scriptable.editableVariables.find(name) != scriptable.editableVariables.end()) {
							if (scriptable.editableVariables[name].first == type) {
								widget->setValue(std::get<int>(scriptable.editableVariables[name].second));
							}
							else {
								scriptable.editableVariables.erase(name);
							}
						}
						
						m_widgetToEditableVariableName[widget] = name;
						
						connect(widget, &IntegerWidget::valueChanged, this, &ScriptableComponentWidget::onEditableVariableChanged);
						
						editableVariablesWidget->layout()->addWidget(widget);
					}
					else if (type == "Uint32") {
						IntegerWidget* widget = new IntegerWidget(m_globalInfo, name);
						widget->setValue(std::get<int>(std::get<2>(editableVariable)));
						if (scriptable.editableVariables.find(name) != scriptable.editableVariables.end()) {
							if (scriptable.editableVariables[name].first == type) {
								widget->setValue(std::get<int>(scriptable.editableVariables[name].second));
							}
							else {
								scriptable.editableVariables.erase(name);
							}
						}
						
						m_widgetToEditableVariableName[widget] = name;
						
						connect(widget, &IntegerWidget::valueChanged, this, &ScriptableComponentWidget::onEditableVariableChanged);
						
						editableVariablesWidget->layout()->addWidget(widget);
					}
					else if (type == "Uint64") {
						IntegerWidget* widget = new IntegerWidget(m_globalInfo, name);
						widget->setValue(std::get<int>(std::get<2>(editableVariable)));
						if (scriptable.editableVariables.find(name) != scriptable.editableVariables.end()) {
							if (scriptable.editableVariables[name].first == type) {
								widget->setValue(std::get<int>(scriptable.editableVariables[name].second));
							}
							else {
								scriptable.editableVariables.erase(name);
							}
						}
						
						m_widgetToEditableVariableName[widget] = name;
						
						connect(widget, &IntegerWidget::valueChanged, this, &ScriptableComponentWidget::onEditableVariableChanged);
						
						editableVariablesWidget->layout()->addWidget(widget);
					}
					else if (type == "Float32") {
						ScalarWidget* widget = new ScalarWidget(m_globalInfo, name);
						widget->setValue(static_cast<float>(std::get<double>(std::get<2>(editableVariable))));
						if (scriptable.editableVariables.find(name) != scriptable.editableVariables.end()) {
							if (scriptable.editableVariables[name].first == type) {
								widget->setValue(std::get<double>(scriptable.editableVariables[name].second));
							}
							else {
								scriptable.editableVariables.erase(name);
							}
						}
						
						m_widgetToEditableVariableName[widget] = name;
						
						connect(widget, &ScalarWidget::valueChanged, this, &ScriptableComponentWidget::onEditableVariableChanged);
						
						editableVariablesWidget->layout()->addWidget(widget);
					}
					else if (type == "Float64") {
						ScalarWidget* widget = new ScalarWidget(m_globalInfo, name);
						widget->setValue(std::get<double>(std::get<2>(editableVariable)));
						if (scriptable.editableVariables.find(name) != scriptable.editableVariables.end()) {
							if (scriptable.editableVariables[name].first == type) {
								widget->setValue(std::get<double>(scriptable.editableVariables[name].second));
							}
							else {
								scriptable.editableVariables.erase(name);
							}
						}
						
						m_widgetToEditableVariableName[widget] = name;
						
						connect(widget, &ScalarWidget::valueChanged, this, &ScriptableComponentWidget::onEditableVariableChanged);
						
						editableVariablesWidget->layout()->addWidget(widget);
					}
					else if (type == "String") {
						StringWidget* widget = new StringWidget(m_globalInfo, name);
						widget->setText(std::get<std::string>(std::get<2>(editableVariable)));
						if (scriptable.editableVariables.find(name) != scriptable.editableVariables.end()) {
							if (scriptable.editableVariables[name].first == type) {
								widget->setText(std::get<std::string>(scriptable.editableVariables[name].second));
							}
							else {
								scriptable.editableVariables.erase(name);
							}
						}
						
						m_widgetToEditableVariableName[widget] = name;
						
						connect(widget, &StringWidget::valueChanged, this, &ScriptableComponentWidget::onEditableVariableChanged);
						
						editableVariablesWidget->layout()->addWidget(widget);
					}
					else if (type == "Vector2") {
						Vector2Widget* widget = new Vector2Widget(m_globalInfo, name);
						widget->setValue(std::get<nml::vec2>(std::get<2>(editableVariable)));
						if (scriptable.editableVariables.find(name) != scriptable.editableVariables.end()) {
							if (scriptable.editableVariables[name].first == type) {
								widget->setValue(std::get<nml::vec2>(scriptable.editableVariables[name].second));
							}
							else {
								scriptable.editableVariables.erase(name);
							}
						}
						
						m_widgetToEditableVariableName[widget] = name;
						
						connect(widget, &Vector2Widget::valueChanged, this, &ScriptableComponentWidget::onEditableVariableChanged);
						
						editableVariablesWidget->layout()->addWidget(widget);
					}
					else if (type == "Vector3") {
						Vector3Widget* widget = new Vector3Widget(m_globalInfo, name);
						widget->setValue(std::get<nml::vec3>(std::get<2>(editableVariable)));
						if (scriptable.editableVariables.find(name) != scriptable.editableVariables.end()) {
							if (scriptable.editableVariables[name].first == type) {
								widget->setValue(std::get<nml::vec3>(scriptable.editableVariables[name].second));
							}
							else {
								scriptable.editableVariables.erase(name);
							}
						}
						
						m_widgetToEditableVariableName[widget] = name;
						
						connect(widget, &Vector3Widget::valueChanged, this, &ScriptableComponentWidget::onEditableVariableChanged);
						
						editableVariablesWidget->layout()->addWidget(widget);
					}
					else if (type == "Vector4") {
						Vector4Widget* widget = new Vector4Widget(m_globalInfo, name);
						widget->setValue(std::get<nml::vec4>(std::get<2>(editableVariable)));
						if (scriptable.editableVariables.find(name) != scriptable.editableVariables.end()) {
							if (scriptable.editableVariables[name].first == type) {
								widget->setValue(std::get<nml::vec4>(scriptable.editableVariables[name].second));
							}
							else {
								scriptable.editableVariables.erase(name);
							}
						}
						
						m_widgetToEditableVariableName[widget] = name;
						
						connect(widget, &Vector4Widget::valueChanged, this, &ScriptableComponentWidget::onEditableVariableChanged);
						
						editableVariablesWidget->layout()->addWidget(widget);
					}
					else if (type == "Quaternion") {
						QuaternionWidget* widget = new QuaternionWidget(m_globalInfo, name);
						widget->setValue(std::get<nml::quat>(std::get<2>(editableVariable)));
						if (scriptable.editableVariables.find(name) != scriptable.editableVariables.end()) {
							if (scriptable.editableVariables[name].first == type) {
								widget->setValue(std::get<nml::quat>(scriptable.editableVariables[name].second));
							}
							else {
								scriptable.editableVariables.erase(name);
							}
						}
						
						m_widgetToEditableVariableName[widget] = name;
						
						connect(widget, &QuaternionWidget::valueChanged, this, &ScriptableComponentWidget::onEditableVariableChanged);
						
						editableVariablesWidget->layout()->addWidget(widget);
					}
					else {
						m_globalInfo.logger.addLog(LogLevel::Warning, "In script \"" + scriptable.scriptName + "\", found variable \"" + name + "\" with an unsupported Editable Script Variable type (\"" + type + "\").");
					}
				}
			}
		}
	}

	if (editableVariablesWidget->layout()->count() == 1) {
		editableVariablesWidget->hide();
	}
	else {
		editableVariablesWidget->show();
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
	updateEditableVariablesWidget(newScriptable);

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

void ScriptableComponentWidget::onCurrentScriptChanged(const QString& path) {
	if (std::filesystem::exists(path.toStdString())) {
		if ((m_globalInfo.currentEntityID != NO_ENTITY) && m_globalInfo.entities[m_globalInfo.currentEntityID].scriptable.has_value()) {
			updateEditableVariablesWidget(m_globalInfo.entities[m_globalInfo.currentEntityID].scriptable.value());
		}
	}
}

void ScriptableComponentWidget::onEditableVariableChanged() {
	Scriptable newScriptable = m_globalInfo.entities[m_globalInfo.currentEntityID].scriptable.value();
	newScriptable.editableVariables.clear();

	QObject* senderWidget = sender();
	for (const auto& widgetToEditableVariableName : m_widgetToEditableVariableName) {
		if (senderWidget == widgetToEditableVariableName.first) {
			for (const auto& editableVariable : m_editableVariables) {
				std::string name = std::get<0>(editableVariable);
				std::string type = std::get<1>(editableVariable);
				if (type != "Unknown") {
					if (type == "Boolean") {
						BooleanWidget* widget = static_cast<BooleanWidget*>(senderWidget);
						newScriptable.editableVariables[name] = { type, widget->getValue() };
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
						newScriptable.editableVariables[name] = { type, widget->getValue() };
					}
					else if ((type == "Float32") ||
						(type == "Float64")) {
						ScalarWidget* widget = static_cast<ScalarWidget*>(senderWidget);
						newScriptable.editableVariables[name] = { type, widget->getValue() };
					}
					else if (type == "String") {
						StringWidget* widget = static_cast<StringWidget*>(senderWidget);
						newScriptable.editableVariables[name] = { type, widget->getText() };
					}
					else if (type == "Vector2") {
						Vector2Widget* widget = static_cast<Vector2Widget*>(senderWidget);
						newScriptable.editableVariables[name] = { type, widget->getValue() };
					}
					else if (type == "Vector3") {
						Vector3Widget* widget = static_cast<Vector3Widget*>(senderWidget);
						newScriptable.editableVariables[name] = { type, widget->getValue() };
					}
					else if (type == "Vector4") {
						Vector4Widget* widget = static_cast<Vector4Widget*>(senderWidget);
						newScriptable.editableVariables[name] = { type, widget->getValue() };
					}
					else if (type == "Quaternion") {
						QuaternionWidget* widget = static_cast<QuaternionWidget*>(senderWidget);
						newScriptable.editableVariables[name] = { type, widget->getValue() };
					}
				}
			}
		}
	}
	updateComponent(m_globalInfo.currentEntityID, &newScriptable);
}
