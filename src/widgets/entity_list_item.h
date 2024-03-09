#pragma once
#include "../common/common.h"
#include <QListWidgetItem>

class EntityListItem : public QListWidgetItem {
public:
	EntityListItem(GlobalInfo& globalInfo, EntityID _entityID);

private:
	GlobalInfo& m_globalInfo;

public:
	EntityID entityID;
};