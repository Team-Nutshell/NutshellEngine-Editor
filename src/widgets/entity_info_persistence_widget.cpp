#include "entity_info_persistence_widget.h"
#include "../undo_commands/change_entity_persistence_command.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QSignalBlocker>

EntityInfoPersistenceWidget::EntityInfoPersistenceWidget(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setLayout(new QHBoxLayout());
	layout()->setAlignment(Qt::AlignmentFlag::AlignLeft);
	layout()->setContentsMargins(0, 0, 0, 0);
	m_persistenceCheckBox = std::make_unique<QCheckBox>();
	layout()->addWidget(m_persistenceCheckBox.get());
	layout()->addWidget(new QLabel("Is Persistent"));

	connect(m_persistenceCheckBox.get(), &QCheckBox::stateChanged, this, &EntityInfoPersistenceWidget::onStateChanged);
	connect(&m_globalInfo.signalEmitter, &SignalEmitter::selectEntitySignal, this, &EntityInfoPersistenceWidget::onSelectEntity);
	connect(&m_globalInfo.signalEmitter, &SignalEmitter::changeEntityPersistenceSignal, this, &EntityInfoPersistenceWidget::onChangeEntityPersistence);
}

void EntityInfoPersistenceWidget::onSelectEntity() {
	if (m_globalInfo.currentEntityID != NO_ENTITY) {
		{
			const QSignalBlocker signalBlocker(m_persistenceCheckBox.get());
			m_persistenceCheckBox->setChecked(m_globalInfo.entities[m_globalInfo.currentEntityID].isPersistent);
		}
	}
}

void EntityInfoPersistenceWidget::onStateChanged(int state) {
	m_globalInfo.undoStack->push(new ChangeEntityPersistenceCommand(m_globalInfo, m_globalInfo.currentEntityID, Qt::CheckState(state) == Qt::CheckState::Checked));
}

void EntityInfoPersistenceWidget::onChangeEntityPersistence(EntityID entityID, bool isPersistent) {
	if (entityID == m_globalInfo.currentEntityID) {
		{
			const QSignalBlocker signalBlocker(m_persistenceCheckBox.get());
			m_persistenceCheckBox->setChecked(isPersistent);
		}
	}
}
