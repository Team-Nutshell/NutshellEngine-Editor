#include "resource_panel.h"
#include <QVBoxLayout>
#include <QLabel>

ResourcePanel::ResourcePanel(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setLayout(new QVBoxLayout());
	layout()->setContentsMargins(2, 2, 2, 2);
	layout()->addWidget(new QLabel("Assets"));
	assetList = std::make_unique<AssetList>(m_globalInfo);
	layout()->addWidget(assetList.get());
}