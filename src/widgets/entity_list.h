#pragma once
#include "../common/global_info.h"
#include "entity_list_item.h"
#include "entity_list_menu.h"
#include <QListWidget>
#include <QPoint>
#include <QKeyEvent>
#include <string>

class EntityList : public QListWidget {
	Q_OBJECT
public:
	EntityList(GlobalInfo& globalInfo);

private:
	EntityListItem* findItemWithEntityID(EntityID entityID);

private:
	GlobalInfo& m_globalInfo;

private slots:
	void onCreateEntity(EntityID entityID);
	void onDestroyEntity(EntityID entityID);
	void onSelectEntity();
	void onChangeEntityName(EntityID entityID, const std::string& name);
	void onToggleCurrentEntityVisibility(bool isVisible);
	void showMenu(const QPoint& pos);
	void onItemSelectionChanged();
	void keyPressEvent(QKeyEvent* event);

public:
	EntityListMenu* menu;
};