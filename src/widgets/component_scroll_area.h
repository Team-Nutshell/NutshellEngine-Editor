#pragma once
#include "../common/global_info.h"
#include "component_list.h"
#include <QScrollArea>

class ComponentScrollArea : public QScrollArea {
	Q_OBJECT
public:
	ComponentScrollArea(GlobalInfo& globalInfo);

private:
	GlobalInfo& m_globalInfo;

public:
	ComponentList* componentList;
};