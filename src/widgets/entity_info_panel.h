#pragma once
#include "../common/global_info.h"
#include "entity_info_name_widget.h"
#include "entity_info_persistence_widget.h"
#include "component_scroll_area.h"
#include <QWidget>

class EntityInfoPanel : public QWidget {
	Q_OBJECT
public:
	EntityInfoPanel(GlobalInfo& globalInfo);

private:
	GlobalInfo& m_globalInfo;

public:
	EntityInfoNameWidget* entityInfoNameWidget;
	EntityInfoPersistenceWidget* entityInfoPersistenceWidget;
	ComponentScrollArea* componentScrollArea;
};