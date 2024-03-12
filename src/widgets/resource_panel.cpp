#include "resource_panel.h"
#include <QVBoxLayout>
#include <QLabel>

ResourcePanel::ResourcePanel(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setLayout(new QVBoxLayout());
	layout()->setContentsMargins(2, 2, 2, 2);
	layout()->addWidget(new QLabel("Assets"));
	assetList = new AssetList(m_globalInfo);
	layout()->addWidget(assetList);
}