#pragma once
#include "../common/common.h"
#include "entity_info_name_widget.h"
#include "entity_info_persistence_widget.h"
#include "component_scroll_area.h"
#include <QWidget>
#include <memory>

class EntityInfoPanel : public QWidget {
	Q_OBJECT
public:
	EntityInfoPanel(GlobalInfo& globalInfo);

private slots:
	void onSelectEntity();

private:
	GlobalInfo& m_globalInfo;

	std::unique_ptr<EntityInfoNameWidget> m_entityInfoNameWidget;
	std::unique_ptr<EntityInfoPersistenceWidget> m_entityInfoPersistenceWidget;
	std::unique_ptr<ComponentScrollArea> m_componentScrollArea;
};