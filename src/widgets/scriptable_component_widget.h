#pragma once
#include "../common/global_info.h"
#include "combo_box_widget.h"
#include <QWidget>
#include <QPushButton>
#include <QFileSystemWatcher>
#include <unordered_map>

class ScriptableComponentWidget : public QWidget {
	Q_OBJECT
public:
	ScriptableComponentWidget(GlobalInfo& globalInfo);

private:
	void updateWidgets(const Scriptable& scriptable);
	void updateComponent(EntityID entityID, Component* component);

	std::vector<std::string> getScriptEntries();

private slots:
	void onEntitySelected();
	void onEntityScriptableAdded(EntityID entityID);
	void onEntityScriptableRemoved(EntityID entityID);
	void onEntityScriptableChanged(EntityID entityID, const Scriptable& scriptable);
	void onElementChanged(const std::string& element);
	void onOpenCodeEditorButtonClicked();
	void onDirectoryChanged(const QString& path);

private:
	GlobalInfo& m_globalInfo;

	std::unordered_map<std::string, std::string> m_scriptToPath;

	QFileSystemWatcher m_scriptsDirectoryWatcher;

public:
	ComboBoxWidget* scriptNameWidget;
	QPushButton* openCodeEditorButton;
};