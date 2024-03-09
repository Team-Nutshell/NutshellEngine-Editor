#pragma once
#include "../common/common.h"
#include "entity_list.h"
#include <QWidget>
#include <memory>

class EntityPanel : public QWidget {
	Q_OBJECT
public:
	EntityPanel(GlobalInfo& globalInfo);

private:
	GlobalInfo& m_globalInfo;

public:
	std::unique_ptr<EntityList> entityList;
};