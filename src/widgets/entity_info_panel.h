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

private slots:
	void onEntitySelected();

private:
	GlobalInfo& m_globalInfo;

	EntityInfoNameWidget* m_entityInfoNameWidget;
	EntityInfoPersistenceWidget* m_entityInfoPersistenceWidget;
	ComponentScrollArea* m_componentScrollArea;
};