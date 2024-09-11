#pragma once
#include "../common/global_info.h"
#include "combo_box_widget.h"
#include <QWidget>
#include <QPushButton>
#include <QFileSystemWatcher>
#include <string>
#include <vector>
#include <tuple>
#include <variant>
#include <unordered_map>

class ScriptableComponentWidget : public QWidget {
	Q_OBJECT
public:
	ScriptableComponentWidget(GlobalInfo& globalInfo);

private:
	void updateWidgets(const Scriptable& scriptable);
	void updateComponent(EntityID entityID, Component* component);

	std::vector<std::string> getScriptEntries();

	std::tuple<std::string, std::string, EditableScriptVariableValue> parseVariableLineTokens(const std::vector<std::string>& tokens, bool usingNamespaceStd, bool usingNamespaceNtshEngnMath);
	void updateEditableVariablesWidget(Scriptable& scriptable);

private slots:
	void onEntitySelected();
	void onEntityScriptableAdded(EntityID entityID);
	void onEntityScriptableRemoved(EntityID entityID);
	void onEntityScriptableChanged(EntityID entityID, const Scriptable& scriptable);
	void onElementChanged(const std::string& element);
	void onOpenCodeEditorButtonClicked();
	void onDirectoryChanged(const QString& path);
	void onCurrentScriptChanged(const QString& path);
	void onEditableVariableChanged();

private:
	GlobalInfo& m_globalInfo;

	std::unordered_map<std::string, std::string> m_scriptToPath;

	std::vector<std::tuple<std::string, std::string, EditableScriptVariableValue>> m_editableVariables;
	std::unordered_map<QWidget*, std::string> m_widgetToEditableVariableName;

	QFileSystemWatcher m_scriptsDirectoryWatcher;
	QFileSystemWatcher m_currentScriptWatcher;

public:
	ComboBoxWidget* scriptNameWidget;
	QPushButton* openCodeEditorButton;
	QWidget* editableVariablesWidget;
};