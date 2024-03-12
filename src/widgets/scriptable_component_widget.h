#pragma once
#include "../common/common.h"
#include "file_selector_widget.h"
#include <QWidget>

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
	void onStringUpdated(const std::string& string);

private:
	GlobalInfo& m_globalInfo;

public:
	FileSelectorWidget* scriptPathWidget;
};