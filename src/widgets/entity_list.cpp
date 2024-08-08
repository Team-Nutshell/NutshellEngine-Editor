#include "entity_list.h"
#include "../common/save_title_changer.h"
#include "../undo_commands/destroy_entities_command.h"
#include "../widgets/main_window.h"
#include <QSizePolicy>
#include <QLabel>
#include <QSignalBlocker>
#include <QFont>
#include <algorithm>
#include <iterator>

EntityList::EntityList(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	QSizePolicy sizePolicy;
	sizePolicy.setHorizontalPolicy(QSizePolicy::Policy::Ignored);
	sizePolicy.setVerticalPolicy(QSizePolicy::Policy::Expanding);
	setSizePolicy(sizePolicy);
	menu = new EntityListMenu(m_globalInfo);
	setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
	setSelectionMode(QAbstractItemView::SelectionMode::NoSelection);
	
	connect(this, &QListWidget::customContextMenuRequested, this, &EntityList::showMenu);
	connect(this, &QListWidget::itemPressed, this, &EntityList::onItemPressed);
	connect(&globalInfo.signalEmitter, &SignalEmitter::createEntitySignal, this, &EntityList::onCreateEntity);
	connect(&globalInfo.signalEmitter, &SignalEmitter::destroyEntitySignal, this, &EntityList::onDestroyEntity);
	connect(&globalInfo.signalEmitter, &SignalEmitter::selectEntitySignal, this, &EntityList::onEntitySelected);
	connect(&globalInfo.signalEmitter, &SignalEmitter::changeEntityNameSignal, this, &EntityList::onChangeEntityName);
	connect(&globalInfo.signalEmitter, &SignalEmitter::toggleEntityVisibilitySignal, this, &EntityList::onToggleEntityVisibility);
}

EntityListItem* EntityList::findItemWithEntityID(EntityID entityID) {
	for (int i = 0; i < count(); i++) {
		EntityListItem* entityListItem = static_cast<EntityListItem*>(item(i));
		if (entityListItem->entityID == entityID) {
			return entityListItem;
		}
	}

	return nullptr;
}

void EntityList::updateSelection() {
	for (int i = 0; i < count(); i++) {
		EntityListItem* entityListItem = static_cast<EntityListItem*>(item(i));
		if (m_globalInfo.currentEntityID == entityListItem->entityID) {
			entityListItem->setBackground(m_currentSelectionColor);
		}
		else if (m_globalInfo.otherSelectedEntityIDs.find(entityListItem->entityID) != m_globalInfo.otherSelectedEntityIDs.end()) {
			entityListItem->setBackground(m_multiSelectionColor);
		}
		else {
			entityListItem->setBackground(QBrush());
		}
	}
}

void EntityList::onCreateEntity(EntityID entityID) {
	addItem(new EntityListItem(m_globalInfo, entityID));

	SaveTitleChanger::change(m_globalInfo.mainWindow);
}

void EntityList::onDestroyEntity(EntityID entityID) {
	takeItem(row(findItemWithEntityID(entityID)));

	SaveTitleChanger::change(m_globalInfo.mainWindow);
}

void EntityList::onEntitySelected() {
	updateSelection();
}

void EntityList::onChangeEntityName(EntityID entityID, const std::string& name) {
	findItemWithEntityID(entityID)->setText(QString::fromStdString(name));

	SaveTitleChanger::change(m_globalInfo.mainWindow);
}

void EntityList::onToggleEntityVisibility(EntityID entityID, bool isVisible) {
	QFont font = findItemWithEntityID(entityID)->font();
	if (isVisible) {
		font.setItalic(false);
	}
	else {
		font.setItalic(true);
	}
	findItemWithEntityID(entityID)->setFont(font);
}

void EntityList::showMenu(const QPoint& pos) {
	if (!itemAt(pos)) {
		menu->deleteEntityAction->setEnabled(false);
	}
	else {
		EntityListItem* entityListItem = static_cast<EntityListItem*>(itemAt(pos));
		m_globalInfo.currentEntityID = entityListItem->entityID;
		menu->deleteEntityAction->setEnabled(true);
	}
	menu->popup(QCursor::pos());
}

void EntityList::onItemPressed(QListWidgetItem* item) {
	EntityListItem* entityListItem = static_cast<EntityListItem*>(item);
	if (m_globalInfo.currentEntityID == entityListItem->entityID) {
		return;
	}

	if (QGuiApplication::keyboardModifiers() == Qt::ShiftModifier) {
		if (m_globalInfo.currentEntityID != NO_ENTITY) {
			m_globalInfo.otherSelectedEntityIDs.insert(m_globalInfo.currentEntityID);
		}
		m_globalInfo.otherSelectedEntityIDs.erase(entityListItem->entityID);
		m_globalInfo.currentEntityID = entityListItem->entityID;
	}
	else if (QGuiApplication::keyboardModifiers() == Qt::ControlModifier) {
		m_globalInfo.otherSelectedEntityIDs.erase(entityListItem->entityID);
	}
	else {
		m_globalInfo.otherSelectedEntityIDs.clear();
		m_globalInfo.currentEntityID = entityListItem->entityID;
	}
	emit m_globalInfo.signalEmitter.selectEntitySignal();
}

void EntityList::keyPressEvent(QKeyEvent* event) {
	if (m_globalInfo.currentEntityID != NO_ENTITY) {
		int currentSelectionIndex = row(findItemWithEntityID((m_globalInfo.currentEntityID)));
		if (event->key() == Qt::Key::Key_Delete) {
			std::vector<EntityID> entitiesToDestroy = { m_globalInfo.currentEntityID };
			std::copy(m_globalInfo.otherSelectedEntityIDs.begin(), m_globalInfo.otherSelectedEntityIDs.end(), std::back_inserter(entitiesToDestroy));
			m_globalInfo.currentEntityID = NO_ENTITY;
			m_globalInfo.otherSelectedEntityIDs.clear();
			m_globalInfo.undoStack->push(new DestroyEntitiesCommand(m_globalInfo, entitiesToDestroy));
		}
		else if (event->key() == Qt::Key::Key_Up) {
			if (currentSelectionIndex == 0) {
				EntityListItem* entityListItem = static_cast<EntityListItem*>(item(count() - 1));
				m_globalInfo.currentEntityID = entityListItem->entityID;
			}
			else {
				EntityListItem* entityListItem = static_cast<EntityListItem*>(item(currentSelectionIndex - 1));
				m_globalInfo.currentEntityID = entityListItem->entityID;
			}
		}
		else if (event->key() == Qt::Key::Key_Down) {
			if (currentSelectionIndex == (count() - 1)) {
				EntityListItem* entityListItem = static_cast<EntityListItem*>(item(0));
				m_globalInfo.currentEntityID = entityListItem->entityID;
			}
			else {
				EntityListItem* entityListItem = static_cast<EntityListItem*>(item(currentSelectionIndex + 1));
				m_globalInfo.currentEntityID = entityListItem->entityID;
			}
		}
		emit m_globalInfo.signalEmitter.selectEntitySignal();
	}
}