#pragma once
#include "../common/global_info.h"
#include "asset_info_name_widget.h"
#include "asset_info_scroll_area.h"
#include <QWidget>

class AssetInfoPanel : public QWidget {
	Q_OBJECT
public:
	AssetInfoPanel(GlobalInfo& globalInfo);

private:
	GlobalInfo& m_globalInfo;

public:
	AssetInfoNameWidget* assetInfoNameWidget;
	AssetInfoScrollArea* assetInfoScrollArea;
};