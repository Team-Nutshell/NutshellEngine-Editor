#pragma once
#include "../common/global_info.h"
#include "asset_panel.h"
#include "script_panel.h"
#include <QSplitter>
#include <QLabel>

class ResourceSplitter : public QSplitter {
	Q_OBJECT
public:
	ResourceSplitter(GlobalInfo& globalInfo);

private:
	GlobalInfo& m_globalInfo;

public:
	AssetPanel* assetPanel;
	ScriptPanel* scriptPanel;
};