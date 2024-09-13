#include "entity_info_panel.h"
#include <QHBoxLayout>
#include <QLabel>

EntityInfoPanel::EntityInfoPanel(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	resize(215, height());

	setLayout(new QVBoxLayout());
	layout()->setAlignment(Qt::AlignmentFlag::AlignTop);
	layout()->setContentsMargins(0, 2, 2, 2);
	layout()->addWidget(new QLabel("Entity Info"));
	entityInfoNameWidget = new EntityInfoNameWidget(m_globalInfo);
	entityInfoNameWidget->hide();
	layout()->addWidget(entityInfoNameWidget);
	entityInfoPersistenceWidget = new EntityInfoPersistenceWidget(m_globalInfo);
	entityInfoPersistenceWidget->hide();
	layout()->addWidget(entityInfoPersistenceWidget);
	componentScrollArea = new ComponentScrollArea(m_globalInfo);
	componentScrollArea->hide();
	layout()->addWidget(componentScrollArea);

	connect(&m_globalInfo.signalEmitter, &SignalEmitter::selectEntitySignal, this, &EntityInfoPanel::onEntitySelected);
}

void EntityInfoPanel::onEntitySelected() {
	if (m_globalInfo.currentEntityID != NO_ENTITY) {
		entityInfoNameWidget->show();
		entityInfoPersistenceWidget->show();
		componentScrollArea->show();
	}
	else {
		entityInfoNameWidget->hide();
		entityInfoPersistenceWidget->hide();
		componentScrollArea->hide();
	}
}
