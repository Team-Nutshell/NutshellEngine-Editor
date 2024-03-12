#include "entity_info_panel.h"
#include <QHBoxLayout>
#include <QLabel>

EntityInfoPanel::EntityInfoPanel(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	resize(190, height());

	setLayout(new QVBoxLayout());
	layout()->setAlignment(Qt::AlignmentFlag::AlignTop);
	layout()->setContentsMargins(0, 2, 2, 2);
	layout()->addWidget(new QLabel("Entity Info"));
	m_entityInfoNameWidget = new EntityInfoNameWidget(m_globalInfo);
	m_entityInfoNameWidget->hide();
	layout()->addWidget(m_entityInfoNameWidget);
	m_entityInfoPersistenceWidget = new EntityInfoPersistenceWidget(m_globalInfo);
	m_entityInfoPersistenceWidget->hide();
	layout()->addWidget(m_entityInfoPersistenceWidget);
	m_componentScrollArea = new ComponentScrollArea(m_globalInfo);
	m_componentScrollArea->hide();
	layout()->addWidget(m_componentScrollArea);

	connect(&m_globalInfo.signalEmitter, &SignalEmitter::selectEntitySignal, this, &EntityInfoPanel::onSelectEntity);
}

void EntityInfoPanel::onSelectEntity() {
	if (m_globalInfo.currentEntityID != NO_ENTITY) {
		m_entityInfoNameWidget->show();
		m_entityInfoPersistenceWidget->show();
		m_componentScrollArea->show();
	}
	else {
		m_entityInfoNameWidget->hide();
		m_entityInfoPersistenceWidget->hide();
		m_componentScrollArea->hide();
	}
}
