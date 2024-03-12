#pragma once
#include "../common/common.h"
#include "entity_list.h"
#include <QWidget>

class EntityPanel : public QWidget {
	Q_OBJECT
public:
	EntityPanel(GlobalInfo& globalInfo);

private:
	GlobalInfo& m_globalInfo;

public:
	EntityList* entityList;
};