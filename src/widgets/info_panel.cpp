#include "info_panel.h"
#include <QHBoxLayout>
#include <QLabel>

InfoPanel::InfoPanel(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	resize(215, height());

	setLayout(new QVBoxLayout());
	layout()->setAlignment(Qt::AlignmentFlag::AlignTop);
	layout()->setContentsMargins(0, 2, 2, 2);
	layout()->addWidget(new QLabel(QString::fromStdString(m_globalInfo.localization.getString("info"))));
	entityInfoPanel = new EntityInfoPanel(m_globalInfo);
	entityInfoPanel->hide();
	layout()->addWidget(entityInfoPanel);
	assetInfoPanel = new AssetInfoPanel(m_globalInfo);
	assetInfoPanel->hide();
	layout()->addWidget(assetInfoPanel);

	connect(&m_globalInfo.signalEmitter, &SignalEmitter::selectEntitySignal, this, &InfoPanel::onEntitySelected);
	connect(&m_globalInfo.signalEmitter, &SignalEmitter::selectAssetSignal, this, &InfoPanel::onAssetSelected);
}

void InfoPanel::onEntitySelected() {
	if (m_globalInfo.currentEntityID != NO_ENTITY) {
		entityInfoPanel->show();
		assetInfoPanel->hide();
	}
	else {
		entityInfoPanel->hide();
	}
}

void InfoPanel::onAssetSelected(const std::string& path) {
	if (!path.empty()) {
		entityInfoPanel->hide();
		assetInfoPanel->show();
	}
	else {
		assetInfoPanel->hide();
	}
}
