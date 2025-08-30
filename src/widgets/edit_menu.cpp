#include "edit_menu.h"
#include "../undo_commands/copy_entities_command.h"
#include <QKeySequence>
#include <algorithm>
#include <iterator>

EditMenu::EditMenu(GlobalInfo& globalInfo) : QMenu("&" + QString::fromStdString(globalInfo.localization.getString("header_edit"))), m_globalInfo(globalInfo) {
	m_undoActionAction = m_globalInfo.actionUndoStack->createUndoAction(this, "&" + QString::fromStdString(m_globalInfo.localization.getString("header_edit_action_undo")));
	m_undoActionAction->setShortcut(QKeySequence::fromString("Ctrl+Z"));
	addAction(m_undoActionAction);
	m_redoActionAction = m_globalInfo.actionUndoStack->createRedoAction(this, "&" + QString::fromStdString(m_globalInfo.localization.getString("header_edit_action_redo")));
	m_redoActionAction->setShortcut(QKeySequence::fromString("Ctrl+Y"));
	addAction(m_redoActionAction);
	addSeparator();
	m_undoSelectionAction = m_globalInfo.selectionUndoStack->createUndoAction(this, "&" + QString::fromStdString(m_globalInfo.localization.getString("header_edit_selection_undo")));
	m_undoSelectionAction->setShortcut(QKeySequence::fromString("Ctrl+K"));
	addAction(m_undoSelectionAction);
	m_redoSelectionAction = m_globalInfo.selectionUndoStack->createRedoAction(this, "&" + QString::fromStdString(m_globalInfo.localization.getString("header_edit_selection_redo")));
	m_redoSelectionAction->setShortcut(QKeySequence::fromString("Ctrl+L"));
	addAction(m_redoSelectionAction);
	addSeparator();
	m_copyEntitiesAction = addAction(QString::fromStdString(m_globalInfo.localization.getString("header_edit_copy_entity")), this, &EditMenu::copyEntities);
	m_copyEntitiesAction->setShortcut(QKeySequence::fromString("Ctrl+C"));
	m_copyEntitiesAction->setEnabled(false);
	m_pasteEntitiesAction = addAction(QString::fromStdString(m_globalInfo.localization.getString("header_edit_paste_entity")), this, &EditMenu::pasteEntities);
	m_pasteEntitiesAction->setShortcut(QKeySequence::fromString("Ctrl+V"));
	m_pasteEntitiesAction->setEnabled(false);
	m_duplicateEntitiesAction = addAction(QString::fromStdString(m_globalInfo.localization.getString("header_edit_duplicate_entity")), this, &EditMenu::duplicateEntities);
	m_duplicateEntitiesAction->setShortcut(QKeySequence::fromString("Ctrl+D"));
	m_duplicateEntitiesAction->setEnabled(false);

	connect(&m_globalInfo.signalEmitter, &SignalEmitter::selectEntitySignal, this, &EditMenu::onEntitySelected);
}

void EditMenu::copyEntities() {
	if (m_globalInfo.currentEntityID != NO_ENTITY) {
		m_globalInfo.copiedEntities.clear();
		m_globalInfo.copiedEntities.push_back(m_globalInfo.entities[m_globalInfo.currentEntityID]);
		for (EntityID otherSelectedEntityID : m_globalInfo.otherSelectedEntityIDs) {
			m_globalInfo.copiedEntities.push_back(m_globalInfo.entities[otherSelectedEntityID]);
		}
		m_pasteEntitiesAction->setEnabled(true);
	}
}

void EditMenu::pasteEntities() {
	if (!m_globalInfo.copiedEntities.empty()) {
		m_globalInfo.actionUndoStack->push(new CopyEntitiesCommand(m_globalInfo, m_globalInfo.copiedEntities));
	}
}

void EditMenu::duplicateEntities() {
	if (m_globalInfo.currentEntityID != NO_ENTITY) {
		std::set<EntityID> selectedEntityIDs = m_globalInfo.otherSelectedEntityIDs;
		selectedEntityIDs.insert(m_globalInfo.currentEntityID);

		std::vector<Entity> entitiesToDuplicate;
		for (EntityID selectedEntityID : selectedEntityIDs) {
			entitiesToDuplicate.push_back(m_globalInfo.entities[selectedEntityID]);
		}

		m_globalInfo.actionUndoStack->push(new CopyEntitiesCommand(m_globalInfo, entitiesToDuplicate));
	}
}

void EditMenu::onEntitySelected() {
	if (m_globalInfo.currentEntityID != NO_ENTITY) {
		m_copyEntitiesAction->setEnabled(true);
		m_duplicateEntitiesAction->setEnabled(true);
	}
	else {
		m_copyEntitiesAction->setEnabled(false);
		m_duplicateEntitiesAction->setEnabled(false);
	}

	if (m_globalInfo.otherSelectedEntityIDs.empty()) {
		m_copyEntitiesAction->setText(QString::fromStdString(m_globalInfo.localization.getString("header_edit_copy_entity")));
		m_pasteEntitiesAction->setText(QString::fromStdString(m_globalInfo.localization.getString("header_edit_paste_entity")));
		m_duplicateEntitiesAction->setText(QString::fromStdString(m_globalInfo.localization.getString("header_edit_duplicate_entity")));
	}
	else {
		m_copyEntitiesAction->setText(QString::fromStdString(m_globalInfo.localization.getString("header_edit_copy_entities")));
		m_pasteEntitiesAction->setText(QString::fromStdString(m_globalInfo.localization.getString("header_edit_paste_entities")));
		m_duplicateEntitiesAction->setText(QString::fromStdString(m_globalInfo.localization.getString("header_edit_duplicate_entities")));
	}
}
