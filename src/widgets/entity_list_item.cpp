#include "entity_list_item.h"

EntityListItem::EntityListItem(GlobalInfo& globalInfo, EntityID _entityID) : m_globalInfo(globalInfo), entityID(_entityID) {
	setText(QString::fromStdString(m_globalInfo.entities[entityID].name));
}