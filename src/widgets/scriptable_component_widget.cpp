#include "scriptable_component_widget.h"
#include "component_title_widget.h"
#include "separator_line.h"
#include "../undo_commands/change_entity_component_command.h"
#include <QVBoxLayout>
#include <QSignalBlocker>
#include <fstream>
#include <vector>
#include <string>

ScriptableComponentWidget::ScriptableComponentWidget(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setLayout(new QVBoxLayout());
	layout()->setAlignment(Qt::AlignmentFlag::AlignTop);
	layout()->setContentsMargins(0, 0, 0, 0);
	layout()->addWidget(new ComponentTitleWidget(m_globalInfo, "Scriptable"));
	scriptPathWidget = std::make_unique<FileSelectorWidget>(m_globalInfo, "No script selected", "Select a script");
	layout()->addWidget(scriptPathWidget.get());
	layout()->addWidget(new SeparatorLine(m_globalInfo));

	connect(scriptPathWidget.get(), &FileSelectorWidget::fileSelected, this, &ScriptableComponentWidget::onStringUpdated);
	connect(&globalInfo.signalEmitter, &SignalEmitter::selectEntitySignal, this, &ScriptableComponentWidget::onSelectEntity);
	connect(&globalInfo.signalEmitter, &SignalEmitter::addEntityScriptableSignal, this, &ScriptableComponentWidget::onAddEntityScriptable);
	connect(&globalInfo.signalEmitter, &SignalEmitter::removeEntityScriptableSignal, this, &ScriptableComponentWidget::onRemoveEntityScriptable);
	connect(&globalInfo.signalEmitter, &SignalEmitter::changeEntityScriptableSignal, this, &ScriptableComponentWidget::onChangeEntityScriptable);
}

void ScriptableComponentWidget::updateWidgets(const Scriptable& scriptable) {
	if (scriptable.scriptName != "") {
		scriptPathWidget->filePathLabel->setText(QString::fromStdString(scriptable.scriptName));
	}
	else {
		scriptPathWidget->filePathLabel->setText("No script selected");
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

void ScriptableComponentWidget::onStringUpdated(const std::string& string) {
	Scriptable newScriptable = m_globalInfo.entities[m_globalInfo.currentEntityID].scriptable.value();
	if (sender() == scriptPathWidget.get()) {
		std::fstream scriptFile(string, std::ios::in);
		if (scriptFile.is_open()) {
			std::string scriptContent((std::istreambuf_iterator<char>(scriptFile)), std::istreambuf_iterator<char>());
			size_t scriptNameStartPos = scriptContent.find("NTSHENGN_SCRIPT(");
			if (scriptNameStartPos != std::string::npos) {
				scriptNameStartPos += 16;
				size_t scriptNameLength = scriptContent.substr(scriptNameStartPos).find(")");
				if (scriptNameLength != std::string::npos) {
					newScriptable.scriptPath = string;
					newScriptable.scriptName = scriptContent.substr(scriptNameStartPos, scriptNameLength);
				}
			}
			else {
				if (m_globalInfo.entities[m_globalInfo.currentEntityID].scriptable->scriptName != "") {
					scriptPathWidget->filePathLabel->setText(QString::fromStdString(m_globalInfo.entities[m_globalInfo.currentEntityID].scriptable->scriptName));
				}
				else {
					scriptPathWidget->filePathLabel->setText("No script selected");
				}
				std::cout << "\"" << string << "\" is not a valid Script (missing NTSHENGN_SCRIPT(ScriptName) macro." << std::endl;
			}
		}
	}
	m_globalInfo.undoStack->push(new ChangeEntityComponentCommand(m_globalInfo, m_globalInfo.currentEntityID, "Scriptable", &newScriptable));
}
