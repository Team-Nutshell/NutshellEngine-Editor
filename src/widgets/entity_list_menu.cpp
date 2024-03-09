#include "entity_list_menu.h"
#include "../undo_commands/create_entity_command.h"
#include "../undo_commands/destroy_entity_command.h"

EntityListMenu::EntityListMenu(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	newEntityAction = addAction("New Entity", this, &EntityListMenu::newEntity);
	deleteEntityAction = addAction("Delete Entity", this, &EntityListMenu::deleteEntity);
}

void EntityListMenu::newEntity() {
	m_globalInfo.undoStack->push(new CreateEntityCommand(m_globalInfo, "Entity_" + std::to_string(m_globalInfo.globalEntityID)));
}

void EntityListMenu::deleteEntity() {
	m_globalInfo.undoStack->push(new DestroyEntityCommand(m_globalInfo, m_globalInfo.currentEntityID));
}