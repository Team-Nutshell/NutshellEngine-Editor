#pragma once
#include "../common/global_info.h"
#include "entity_list_item.h"
#include "entity_list_menu.h"
#include <QListWidget>
#include <QPoint>
#include <QKeyEvent>
#include <QDropEvent>
#include <string>

class EntityList : public QListWidget {
	Q_OBJECT
public:
	EntityList(GlobalInfo& globalInfo);

	EntityListItem* findItemWithEntityID(EntityID entityID);

	void updateSelection();

private slots:
	void onEntityCreated(EntityID entityID);
	void onEntityDestroyed(EntityID entityID);
	void onEntitySelected();
	void onEntityNameChanged(EntityID entityID, const std::string& name);
	void onEntityPersistenceChanged(EntityID entityID, bool isPersistent);
	void onEntityVisibilityToggled(EntityID entityID, bool isVisible);
	void showMenu(const QPoint& pos);
	void dropEvent(QDropEvent* event);
	void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
	void keyPressEvent(QKeyEvent* event);
	void keyReleaseEvent(QKeyEvent* event);
	void onLineEditClose(QWidget* lineEdit, QAbstractItemDelegate::EndEditHint hint);

private:
	GlobalInfo& m_globalInfo;

	bool m_moveEntityOrderKeyPressed = false;

	bool m_selfSignal = false;

public:
	EntityListMenu* menu;
};