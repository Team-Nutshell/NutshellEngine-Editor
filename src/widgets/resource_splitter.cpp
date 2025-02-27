#include "resource_splitter.h"
#include <QSplitter>

ResourceSplitter::ResourceSplitter(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setOrientation(Qt::Orientation::Horizontal);
	assetPanel = new AssetPanel(m_globalInfo);
	addWidget(assetPanel);
	scriptPanel = new ScriptPanel(m_globalInfo);
	addWidget(scriptPanel);
}