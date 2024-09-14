#pragma once
#include "../common/global_info.h"
#include "asset_info_list.h"
#include <QScrollArea>

class AssetInfoScrollArea : public QScrollArea {
	Q_OBJECT
public:
	AssetInfoScrollArea(GlobalInfo& globalInfo);

private:
	GlobalInfo& m_globalInfo;

public:
	AssetInfoList* assetInfoList;
};