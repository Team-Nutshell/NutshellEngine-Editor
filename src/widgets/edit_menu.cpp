#include "edit_menu.h"
#include "../undo_commands/copy_entities_command.h"
#include <QKeySequence>
#include <algorithm>
#include <iterator>

EditMenu::EditMenu(GlobalInfo& globalInfo): QMenu("&Edit"), m_globalInfo(globalInfo) {
	m_undoAction = m_globalInfo.undoStack->createUndoAction(this, "&Undo");
	m_undoAction->setShortcut(QKeySequence::fromString("Ctrl+Z"));
	addAction(m_undoAction);
	m_redoAction = m_globalInfo.undoStack->createRedoAction(this, "&Redo");
	m_redoAction->setShortcut(QKeySequence::fromString("Ctrl+Y"));
	addAction(m_redoAction);
	addSeparator();
	m_copyEntitiesAction = addAction("Copy Entity", this, &EditMenu::copyEntities);
	m_copyEntitiesAction->setShortcut(QKeySequence::fromString("Ctrl+C"));
	m_copyEntitiesAction->setEnabled(false);
	m_pasteEntitiesAction = addAction("Paste Entity", this, &EditMenu::pasteEntities);
	m_pasteEntitiesAction->setShortcut(QKeySequence::fromString("Ctrl+V"));
	m_pasteEntitiesAction->setEnabled(false);

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
		m_globalInfo.undoStack->push(new CopyEntitiesCommand(m_globalInfo, m_globalInfo.copiedEntities));
	}
}

void EditMenu::onEntitySelected() {
	if (m_globalInfo.currentEntityID != NO_ENTITY) {
		m_copyEntitiesAction->setEnabled(true);
	}
	else {
		m_copyEntitiesAction->setEnabled(false);
	}

	if (m_globalInfo.otherSelectedEntityIDs.empty()) {
		m_copyEntitiesAction->setText("Copy Entity");
		m_pasteEntitiesAction->setText("Paste Entity");
	}
	else {
		m_copyEntitiesAction->setText("Copy Entities");
		m_pasteEntitiesAction->setText("Paste Entities");
	}
}
