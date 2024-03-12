#pragma once
#include "../common/common.h"
#include "combo_box_widget.h"
#include <QWidget>
#include <QFileSystemWatcher>

class ScriptableComponentWidget : public QWidget {
	Q_OBJECT
public:
	ScriptableComponentWidget(GlobalInfo& globalInfo);

private:
	void updateWidgets(const Scriptable& scriptable);

private slots:
	void onSelectEntity();
	void onAddEntityScriptable(EntityID entityID);
	void onRemoveEntityScriptable(EntityID entityID);
	void onChangeEntityScriptable(EntityID entityID, const Scriptable& scriptable);
	void onElementUpdated(const std::string& element);
	void onDirectoryChanged(const QString& path);

private:
	GlobalInfo& m_globalInfo;

	QFileSystemWatcher m_scriptsDirectoryWatcher;

public:
	ComboBoxWidget* scriptNameWidget;
};