#pragma once
#include "../common/global_info.h"
#include "entity_info_panel.h"
#include "asset_info_panel.h"
#include <QWidget>

class InfoPanel : public QWidget {
	Q_OBJECT
public:
	InfoPanel(GlobalInfo& globalInfo);

private slots:
	void onEntitySelected();
	void onAssetSelected(const std::string& path);

private:
	GlobalInfo& m_globalInfo;

public:
	EntityInfoPanel* entityInfoPanel;
	AssetInfoPanel* assetInfoPanel;
};