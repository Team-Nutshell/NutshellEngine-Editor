#include "entity_list_menu.h"
#include "main_window.h"
#include "../undo_commands/create_entity_command.h"
#include "../undo_commands/destroy_entities_command.h"

EntityListMenu::EntityListMenu(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	renameEntityAction = addAction(QString::fromStdString(m_globalInfo.localization.getString("entity_rename")), this, &EntityListMenu::renameEntity);
	addSeparator();
	newEntityAction = addAction(QString::fromStdString(m_globalInfo.localization.getString("entity_new")), this, &EntityListMenu::newEntity);
	deleteEntityAction = addAction(QString::fromStdString(m_globalInfo.localization.getString("entity_delete")), this, &EntityListMenu::deleteEntity);
}

void EntityListMenu::renameEntity() {
	EntityListItem* entityListItem = m_globalInfo.mainWindow->entityPanel->entityList->findItemWithEntityID(m_globalInfo.currentEntityID);
	entityListItem->setFlags(entityListItem->flags() | Qt::ItemFlag::ItemIsEditable);
	m_globalInfo.mainWindow->entityPanel->entityList->editItem(entityListItem);
}

void EntityListMenu::newEntity() {
	m_globalInfo.actionUndoStack->push(new CreateEntityCommand(m_globalInfo, "Entity_" + std::to_string(m_globalInfo.globalEntityID)));
}

void EntityListMenu::deleteEntity() {
	m_globalInfo.actionUndoStack->push(new DestroyEntitiesCommand(m_globalInfo, { m_globalInfo.currentEntityID }));
}