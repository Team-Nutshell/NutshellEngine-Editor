#include "entity_info_panel.h"
#include <QHBoxLayout>
#include <QLabel>

EntityInfoPanel::EntityInfoPanel(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setLayout(new QVBoxLayout());
	layout()->setAlignment(Qt::AlignmentFlag::AlignTop);
	layout()->setContentsMargins(0, 0, 0, 0);
	entityInfoNameWidget = new EntityInfoNameWidget(m_globalInfo);
	layout()->addWidget(entityInfoNameWidget);
	entityInfoPersistenceWidget = new EntityInfoPersistenceWidget(m_globalInfo);
	layout()->addWidget(entityInfoPersistenceWidget);
	entityGroupsWidget = new EntityGroupsWidget(m_globalInfo);
	collapsableEntityGroupsWidget = new CollapsableWidget(m_globalInfo.localization.getString("entity_groups"), entityGroupsWidget);
	layout()->addWidget(collapsableEntityGroupsWidget);
	componentScrollArea = new ComponentScrollArea(m_globalInfo);
	layout()->addWidget(componentScrollArea);

	connect(&m_globalInfo.signalEmitter, &SignalEmitter::selectEntitySignal, this, &EntityInfoPanel::updateEntityGroupsNumber);
	connect(entityGroupsWidget, &EntityGroupsWidget::updateEntityGroupsSignal, this, &EntityInfoPanel::updateEntityGroupsNumber);
}

void EntityInfoPanel::updateEntityGroupsNumber() {
	if (m_globalInfo.currentEntityID != NO_ENTITY) {
		collapsableEntityGroupsWidget->setText(m_globalInfo.localization.getString("entity_groups") + " (" + std::to_string(m_globalInfo.entities[m_globalInfo.currentEntityID].entityGroups.size()) + ")");
	}
}