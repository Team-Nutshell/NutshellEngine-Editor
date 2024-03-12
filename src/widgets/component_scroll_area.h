#pragma once
#include "../common/common.h"
#include "component_list.h"
#include <QScrollArea>

class ComponentScrollArea : public QScrollArea {
	Q_OBJECT
public:
	ComponentScrollArea(GlobalInfo& globalInfo);

private:
	GlobalInfo& m_globalInfo;

	ComponentList* componentList;
};