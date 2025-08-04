#include "entity_info_persistence_widget.h"
#include "../undo_commands/change_entities_persistence_command.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QSignalBlocker>

EntityInfoPersistenceWidget::EntityInfoPersistenceWidget(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setLayout(new QHBoxLayout());
	layout()->setAlignment(Qt::AlignmentFlag::AlignLeft);
	layout()->setContentsMargins(0, 0, 0, 0);
	m_persistenceCheckBox = new QCheckBox();
	layout()->addWidget(m_persistenceCheckBox);
	layout()->addWidget(new QLabel(QString::fromStdString(m_globalInfo.localization.getString("entity_is_persistent"))));

	connect(m_persistenceCheckBox, &QCheckBox::checkStateChanged, this, &EntityInfoPersistenceWidget::onStateChanged);
	connect(&m_globalInfo.signalEmitter, &SignalEmitter::selectEntitySignal, this, &EntityInfoPersistenceWidget::onEntitySelected);
	connect(&m_globalInfo.signalEmitter, &SignalEmitter::changeEntityPersistenceSignal, this, &EntityInfoPersistenceWidget::onEntityPersistenceChanged);
}

void EntityInfoPersistenceWidget::onEntitySelected() {
	if (m_globalInfo.currentEntityID != NO_ENTITY) {
		{
			const QSignalBlocker signalBlocker(m_persistenceCheckBox);
			m_persistenceCheckBox->setChecked(m_globalInfo.entities[m_globalInfo.currentEntityID].isPersistent);
		}
	}
}

void EntityInfoPersistenceWidget::onStateChanged(int state) {
	if (m_globalInfo.currentEntityID != NO_ENTITY) {
		std::vector<EntityID> entityIDs{ m_globalInfo.currentEntityID };
		std::copy(m_globalInfo.otherSelectedEntityIDs.begin(), m_globalInfo.otherSelectedEntityIDs.end(), std::back_inserter(entityIDs));
		m_globalInfo.actionUndoStack->push(new ChangeEntitiesPersistenceCommand(m_globalInfo, entityIDs, Qt::CheckState(state) == Qt::CheckState::Checked));
	}
}

void EntityInfoPersistenceWidget::onEntityPersistenceChanged(EntityID entityID, bool isPersistent) {
	if (entityID == m_globalInfo.currentEntityID) {
		{
			const QSignalBlocker signalBlocker(m_persistenceCheckBox);
			m_persistenceCheckBox->setChecked(isPersistent);
		}
	}
}
