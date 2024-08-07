#include "edit_menu.h"
#include "../undo_commands/copy_entity_command.h"
#include <QKeySequence>

EditMenu::EditMenu(GlobalInfo& globalInfo): QMenu("&Edit"), m_globalInfo(globalInfo) {
	m_undoAction = m_globalInfo.undoStack->createUndoAction(this, "&Undo");
	m_undoAction->setShortcut(QKeySequence::fromString("Ctrl+Z"));
	addAction(m_undoAction);
	m_redoAction = m_globalInfo.undoStack->createRedoAction(this, "&Redo");
	m_redoAction->setShortcut(QKeySequence::fromString("Ctrl+Y"));
	addAction(m_redoAction);
	addSeparator();
	m_copyEntityAction = addAction("Copy Entity", this, &EditMenu::copyEntity);
	m_copyEntityAction->setShortcut(QKeySequence::fromString("Ctrl+C"));
	m_copyEntityAction->setEnabled(false);
	m_pasteEntityAction = addAction("Paste Entity", this, &EditMenu::pasteEntity);
	m_pasteEntityAction->setShortcut(QKeySequence::fromString("Ctrl+V"));
	m_pasteEntityAction->setEnabled(false);

	connect(&m_globalInfo.signalEmitter, &SignalEmitter::selectEntitySignal, this, &EditMenu::onEntitySelected);
}

void EditMenu::copyEntity() {
	if (m_globalInfo.currentEntityID != NO_ENTITY) {
		m_globalInfo.copiedEntity = m_globalInfo.entities[m_globalInfo.currentEntityID];
		m_pasteEntityAction->setEnabled(true);
	}
}

void EditMenu::pasteEntity() {
	if (m_globalInfo.copiedEntity) {
		m_globalInfo.undoStack->push(new CopyEntityCommand(m_globalInfo, m_globalInfo.copiedEntity.value()));
	}
}

void EditMenu::onEntitySelected() {
	if (m_globalInfo.currentEntityID != NO_ENTITY) {
		m_copyEntityAction->setEnabled(true);
	}
	else {
		m_copyEntityAction->setEnabled(false);
	}
}
