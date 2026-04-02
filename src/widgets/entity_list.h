#pragma once
#include "../common/global_info.h"
#include "entity_list_item.h"
#include "entity_list_menu.h"
#include <QListWidget>
#include <QPoint>
#include <QColor>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QDropEvent>
#include <string>

class EntityList : public QListWidget {
	Q_OBJECT
public:
	EntityList(GlobalInfo& globalInfo);

	EntityListItem* findItemWithEntityID(EntityID entityID);
	std::vector<EntityID> getRowSortedSelectedEntityIDs();

	void updateSelection();

	void resizeFont(int delta);

	void duplicateEntities();

private slots:
	void onEntityCreated(EntityID entityID);
	void onEntityDestroyed(EntityID entityID);
	void onEntitySelected();
	void onEntityNameChanged(EntityID entityID, const std::string& name);
	void onEntityPersistenceChanged(EntityID entityID, bool isPersistent);
	void onEntityVisibilityToggled(EntityID entityID, bool entityIsVisible, bool renderableIsVisible);
	void showMenu(const QPoint& pos);
	void dropEvent(QDropEvent* event);
	void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
	void keyPressEvent(QKeyEvent* event);
	void keyReleaseEvent(QKeyEvent* event);
	void wheelEvent(QWheelEvent* event);
	void onLineEditClose(QWidget* lineEdit, QAbstractItemDelegate::EndEditHint hint);

private:
	GlobalInfo& m_globalInfo;

	bool m_moveEntityOrderKeyPressed = false;

	QColor m_entityNotVisibleColor = QColor(120.0f, 120.0f, 120.0f);
	QColor m_entityRenderableNotVisibleColor = QColor(108.0f, 180.0f, 237.0f);
	QColor m_entityNotVisibleRenderableNotVisibleColor = QColor(75.0f, 148.0f, 250.0f);

public:
	EntityListMenu* menu;

	bool blockSelectionSignal = false;
};