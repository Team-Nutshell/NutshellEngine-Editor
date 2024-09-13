#pragma once
#include "../common/global_info.h"
#include "combo_box_widget.h"
#include <QWidget>
#include <QPushButton>
#include <QFileSystemWatcher>
#include <string>
#include <vector>
#include <variant>
#include <set>
#include <unordered_map>

class ScriptableComponentWidget : public QWidget {
	Q_OBJECT
public:
	ScriptableComponentWidget(GlobalInfo& globalInfo);

private:
	void updateWidgets(const Scriptable& scriptable);
	void updateComponent(EntityID entityID, Component* component);

	std::vector<std::string> getScriptEntries();

	std::pair<std::string, std::pair<std::string, EditableScriptVariableValue>> parseVariableLineTokens(const std::vector<std::string>& tokens, bool usingNamespaceStd, bool usingNamespaceNtshEngnMath);
	void updateEditableScriptVariables(const std::string& scriptName);
	void createEditableScriptVariablesWidget(const std::string& scriptName);
	void updateEditableScriptVariablesWidget(Scriptable& scriptable);

private slots:
	void onEntitySelected();
	void onEntityScriptableAdded(EntityID entityID);
	void onEntityScriptableRemoved(EntityID entityID);
	void onEntityScriptableChanged(EntityID entityID, const Scriptable& scriptable);
	void onElementChanged(const std::string& element);
	void onOpenCodeEditorButtonClicked();
	void onDirectoryChanged(const QString& path);
	void onCurrentScriptChanged(const QString& path);
	void onEditableScriptVariableChanged();

private:
	GlobalInfo& m_globalInfo;

	std::unordered_map<std::string, std::unordered_map<std::string, std::pair<std::string, EditableScriptVariableValue>>> m_editableScriptVariables;
	std::unordered_map<QWidget*, std::string> m_widgetToEditableScriptVariableName;
	std::unordered_map<std::string, QWidget*> m_editableScriptVariableNameToWidget;

	std::unordered_map<std::string, std::string> m_scriptToPath;
	std::set<std::string> m_scriptsNoCase;

	QFileSystemWatcher m_scriptsDirectoryWatcher;
	QFileSystemWatcher m_currentScriptWatcher;

public:
	ComboBoxWidget* scriptNameWidget;
	QPushButton* openCodeEditorButton;
	QWidget* editableScriptVariablesWidget;
};