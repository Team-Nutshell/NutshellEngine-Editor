#pragma once
#include "../common/common.h"
#include "asset_list.h"
#include <QWidget>

class ResourcePanel : public QWidget {
	Q_OBJECT
public:
	ResourcePanel(GlobalInfo& globalInfo);

private:
	GlobalInfo& m_globalInfo;

public:
	AssetList* assetList;
};