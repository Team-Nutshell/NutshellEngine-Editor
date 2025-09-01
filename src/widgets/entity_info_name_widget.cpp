#include "entity_info_name_widget.h"
#include "main_window.h"
#include "../undo_commands/change_entities_name_command.h"

EntityInfoNameWidget::EntityInfoNameWidget(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	connect(this, &QLineEdit::editingFinished, this, &EntityInfoNameWidget::onEditingFinished);
	connect(&globalInfo.signalEmitter, &SignalEmitter::selectEntitySignal, this, &EntityInfoNameWidget::onEntitySelected);
	connect(&globalInfo.signalEmitter, &SignalEmitter::changeEntityNameSignal, this, &EntityInfoNameWidget::onEntityNameChanged);
}

void EntityInfoNameWidget::onEntitySelected() {
	if (m_globalInfo.currentEntityID != NO_ENTITY) {
		setText(QString::fromStdString(m_globalInfo.entities[m_globalInfo.currentEntityID].name));
		m_previousName = m_globalInfo.entities[m_globalInfo.currentEntityID].name;
	}
}

void EntityInfoNameWidget::onEditingFinished() {
	if (m_previousName != text().toStdString()) {
		m_previousName = text().toStdString();
		std::vector<EntityID> entityIDsToRename = m_globalInfo.mainWindow->entityPanel->entityList->getRowSortedSelectedEntityIDs();
		if (!entityIDsToRename.empty()) {
			m_globalInfo.actionUndoStack->push(new ChangeEntitiesNameCommand(m_globalInfo, entityIDsToRename, text().toStdString()));
		}
	}
}

void EntityInfoNameWidget::onEntityNameChanged(EntityID entityID, const std::string& name) {
	if (entityID == m_globalInfo.currentEntityID) {
		setText(QString::fromStdString(name));
		m_previousName = name;
	}
}