#include "component_scroll_area.h"

ComponentScrollArea::ComponentScrollArea(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setWidgetResizable(true);
	componentList = std::make_unique<ComponentList>(m_globalInfo);
	setWidget(componentList.get());
}
