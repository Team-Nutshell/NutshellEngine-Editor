#include "asset_info_scroll_area.h"

AssetInfoScrollArea::AssetInfoScrollArea(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setWidgetResizable(true);
	assetInfoList = new AssetInfoList(m_globalInfo);
	setWidget(assetInfoList);
}
