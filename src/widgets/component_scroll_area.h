#pragma once
#include "../common/common.h"
#include "component_list.h"
#include <QScrollArea>
#include <memory>

class ComponentScrollArea : public QScrollArea {
	Q_OBJECT
public:
	ComponentScrollArea(GlobalInfo& globalInfo);

private:
	GlobalInfo& m_globalInfo;

	std::unique_ptr<ComponentList> componentList;
};