#pragma once
#include "../common/global_info.h"
#include "script_list.h"
#include <QWidget>
#include <QLabel>

class ScriptPanel : public QWidget {
	Q_OBJECT
public:
	ScriptPanel(GlobalInfo& globalInfo);

private:
	GlobalInfo& m_globalInfo;

public:
	QLabel* scriptsLabel;
	ScriptList* scriptList;
};