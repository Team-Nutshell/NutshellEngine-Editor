#include "component_scroll_area.h"

ComponentScrollArea::ComponentScrollArea(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setWidgetResizable(true);
	componentList = new ComponentList(m_globalInfo);
	setWidget(componentList);
}
