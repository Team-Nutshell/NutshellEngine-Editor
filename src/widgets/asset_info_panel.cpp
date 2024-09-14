#include "asset_info_panel.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>

AssetInfoPanel::AssetInfoPanel(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setLayout(new QVBoxLayout());
	layout()->setAlignment(Qt::AlignmentFlag::AlignTop);
	layout()->setContentsMargins(0, 0, 0, 0);
	assetInfoNameWidget = new AssetInfoNameWidget(m_globalInfo);
	layout()->addWidget(assetInfoNameWidget);
	assetInfoScrollArea = new AssetInfoScrollArea(m_globalInfo);
	layout()->addWidget(assetInfoScrollArea);
}