#pragma once
#include "../common/global_info.h"
#include <QListWidgetItem>

class EntityListItem : public QListWidgetItem {
public:
	EntityListItem(GlobalInfo& globalInfo, EntityID _entityID);

private:
	GlobalInfo& m_globalInfo;

public:
	EntityID entityID;
};