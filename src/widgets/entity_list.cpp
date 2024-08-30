#include "entity_list.h"
#include "main_window.h"
#include "../common/save_title_changer.h"
#include "../undo_commands/destroy_entities_command.h"
#include <QSizePolicy>
#include <QLabel>
#include <QSignalBlocker>
#include <QFont>
#include <algorithm>
#include <iterator>
#include <functional>

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
	connect(&globalInfo.signalEmitter, &SignalEmitter::createEntitySignal, this, &EntityList::onEntityCreated);
	connect(&globalInfo.signalEmitter, &SignalEmitter::destroyEntitySignal, this, &EntityList::onEntityDestroyed);
	connect(&globalInfo.signalEmitter, &SignalEmitter::selectEntitySignal, this, &EntityList::onEntitySelected);
	connect(&globalInfo.signalEmitter, &SignalEmitter::changeEntityNameSignal, this, &EntityList::onEntityNameChanged);
	connect(&globalInfo.signalEmitter, &SignalEmitter::toggleEntityVisibilitySignal, this, &EntityList::onEntityVisibilityToggled);
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

void EntityList::onEntityCreated(EntityID entityID) {
	addItem(new EntityListItem(m_globalInfo, entityID));

	SaveTitleChanger::change(m_globalInfo.mainWindow);
}

void EntityList::onEntityDestroyed(EntityID entityID) {
	takeItem(row(findItemWithEntityID(entityID)));

	SaveTitleChanger::change(m_globalInfo.mainWindow);
}

void EntityList::onEntitySelected() {
	updateSelection();
}

void EntityList::onEntityNameChanged(EntityID entityID, const std::string& name) {
	findItemWithEntityID(entityID)->setText(QString::fromStdString(name));

	SaveTitleChanger::change(m_globalInfo.mainWindow);
}

void EntityList::onEntityVisibilityToggled(EntityID entityID, bool isVisible) {
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

void EntityList::onItemPressed(QListWidgetItem* listWidgetItem) {
	EntityListItem* entityListItem = static_cast<EntityListItem*>(listWidgetItem);
	if (m_globalInfo.currentEntityID == entityListItem->entityID) {
		return;
	}

	if (QGuiApplication::keyboardModifiers() == Qt::ShiftModifier) {
		if (m_globalInfo.currentEntityID != NO_ENTITY) {
			int currentEntityIndex = row(findItemWithEntityID(m_globalInfo.currentEntityID));
			int selectionIndex = row(listWidgetItem);
			int startRange = std::min(currentEntityIndex, selectionIndex);
			int endRange = std::max(currentEntityIndex, selectionIndex);
			m_globalInfo.otherSelectedEntityIDs.insert(m_globalInfo.currentEntityID);
			for (int i = startRange; i < endRange; i++) {
				m_globalInfo.otherSelectedEntityIDs.insert(static_cast<EntityListItem*>(item(i))->entityID);
			}
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
		int currentSelectionIndex = row(findItemWithEntityID(m_globalInfo.currentEntityID));
		if (event->key() == Qt::Key::Key_Delete) {
			std::vector<EntityID> entitiesToDestroy = { m_globalInfo.currentEntityID };
			std::copy(m_globalInfo.otherSelectedEntityIDs.begin(), m_globalInfo.otherSelectedEntityIDs.end(), std::back_inserter(entitiesToDestroy));
			m_globalInfo.undoStack->push(new DestroyEntitiesCommand(m_globalInfo, entitiesToDestroy));
			if (count() != 0) {
				if (currentSelectionIndex < count()) {
					EntityListItem* entityListItem = static_cast<EntityListItem*>(item(currentSelectionIndex));
					m_globalInfo.currentEntityID = entityListItem->entityID;
				}
				else if (currentSelectionIndex > count()) {
					EntityListItem* entityListItem = static_cast<EntityListItem*>(item(count() - 1));
					m_globalInfo.currentEntityID = entityListItem->entityID;
				}
				else {
					EntityListItem* entityListItem = static_cast<EntityListItem*>(item(currentSelectionIndex - 1));
					m_globalInfo.currentEntityID = entityListItem->entityID;
				}
			}
			emit m_globalInfo.signalEmitter.selectEntitySignal();
		}
		else if (event->key() == Qt::Key::Key_Up) {
			if (!m_moveEntityOrderKeyPressed) {
				m_globalInfo.clearSelectedEntities();
				if (currentSelectionIndex == 0) {
					EntityListItem* entityListItem = static_cast<EntityListItem*>(item(count() - 1));
					m_globalInfo.currentEntityID = entityListItem->entityID;
				}
				else {
					EntityListItem* entityListItem = static_cast<EntityListItem*>(item(currentSelectionIndex - 1));
					m_globalInfo.currentEntityID = entityListItem->entityID;
				}
			}
			else {
				std::vector<int> entityIndexes = { currentSelectionIndex };
				for (EntityID otherSelectedEntityID : m_globalInfo.otherSelectedEntityIDs) {
					entityIndexes.push_back(row(findItemWithEntityID(otherSelectedEntityID)));
				}
				std::sort(entityIndexes.begin(), entityIndexes.end());
				bool stopMoving = false;
				for (int entityIndex : entityIndexes) {
					EntityListItem* entityListItem = static_cast<EntityListItem*>(takeItem(entityIndex));
					int newPosition = 0;
					if (entityIndex == 0) {
						newPosition = count();
						stopMoving = true;
					}
					else {
						newPosition = entityIndex - 1;
					}
					insertItem(newPosition, entityListItem);

					if (stopMoving) {
						break;
					}
				}
			}
		}
		else if (event->key() == Qt::Key::Key_Down) {
			if (!m_moveEntityOrderKeyPressed) {
				m_globalInfo.clearSelectedEntities();
				if (currentSelectionIndex == (count() - 1)) {
					EntityListItem* entityListItem = static_cast<EntityListItem*>(item(0));
					m_globalInfo.currentEntityID = entityListItem->entityID;
				}
				else {
					EntityListItem* entityListItem = static_cast<EntityListItem*>(item(currentSelectionIndex + 1));
					m_globalInfo.currentEntityID = entityListItem->entityID;
				}
			}
			else {
				std::vector<int> entityIndexes = { currentSelectionIndex };
				for (EntityID otherSelectedEntityID : m_globalInfo.otherSelectedEntityIDs) {
					entityIndexes.push_back(row(findItemWithEntityID(otherSelectedEntityID)));
				}
				std::sort(entityIndexes.begin(), entityIndexes.end(), std::greater<int>());
				bool stopMoving = false;
				for (int entityIndex : entityIndexes) {
					EntityListItem* entityListItem = static_cast<EntityListItem*>(takeItem(entityIndex));
					int newPosition = 0;
					if (entityIndex == count()) {
						newPosition = 0;
						stopMoving = true;
					}
					else {
						newPosition = entityIndex + 1;
					}
					insertItem(newPosition, entityListItem);

					if (stopMoving) {
						break;
					}
				}
			}
		}
		else if (event->key() == Qt::Key::Key_Alt) {
			m_moveEntityOrderKeyPressed = true;
		}
		emit m_globalInfo.signalEmitter.selectEntitySignal();
	}
}

void EntityList::keyReleaseEvent(QKeyEvent* event) {
	if (event->isAutoRepeat()) {
		event->accept();
		return;
	}

	if (event->key() == Qt::Key::Key_Alt) {
		m_moveEntityOrderKeyPressed = false;
	}
}
