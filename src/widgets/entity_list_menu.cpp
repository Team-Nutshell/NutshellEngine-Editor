#include "entity_list_menu.h"
#include "entity_list.h"
#include "main_window.h"
#include "../undo_commands/create_entity_command.h"
#include "../undo_commands/destroy_entities_command.h"

EntityListMenu::EntityListMenu(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	renameAction = addAction(QString::fromStdString(m_globalInfo.localization.getString("entity_rename")), this, &EntityListMenu::renameEntities);
	addSeparator();
	newEntityAction = addAction(QString::fromStdString(m_globalInfo.localization.getString("entity_new")), this, &EntityListMenu::newEntities);
	deleteAction = addAction(QString::fromStdString(m_globalInfo.localization.getString("entity_delete")), this, &EntityListMenu::deleteEntities);
	duplicateAction = addAction(QString::fromStdString(m_globalInfo.localization.getString("entity_duplicate")), this, &EntityListMenu::duplicateEntities);
	duplicateAction->setShortcut(QKeySequence::fromString("Ctrl+D"));
}

void EntityListMenu::renameEntities() {
	EntityListItem* entityListItem = entityList->findItemWithEntityID(m_globalInfo.currentEntityID);
	entityListItem->setFlags(entityListItem->flags() | Qt::ItemFlag::ItemIsEditable);
	entityList->editItem(entityListItem);
}

void EntityListMenu::newEntities() {
	m_globalInfo.actionUndoStack->push(new CreateEntityCommand(m_globalInfo, "Entity_" + std::to_string(m_globalInfo.globalEntityID)));
	QListWidgetItem* item = entityList->item(entityList->count() - 1);
	item->setFlags(item->flags() | Qt::ItemFlag::ItemIsEditable);
	entityList->editItem(item);
}

void EntityListMenu::deleteEntities() {
	m_globalInfo.actionUndoStack->push(new DestroyEntitiesCommand(m_globalInfo, { m_globalInfo.currentEntityID }));
}

void EntityListMenu::duplicateEntities() {
	entityList->duplicateEntities();
}