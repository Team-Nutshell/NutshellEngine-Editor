#include "entity_list.h"
#include "main_window.h"
#include "../common/save_title_changer.h"
#include "../undo_commands/destroy_entities_command.h"
#include "../undo_commands/change_entities_name_command.h"
#include "../undo_commands/select_asset_entities_command.h"
#include <QSizePolicy>
#include <QLabel>
#include <QFont>
#include <QGuiApplication>
#include <algorithm>
#include <iterator>
#include <functional>
#include <utility>

EntityList::EntityList(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	QSizePolicy sizePolicy;
	sizePolicy.setHorizontalPolicy(QSizePolicy::Policy::Ignored);
	sizePolicy.setVerticalPolicy(QSizePolicy::Policy::Expanding);
	setSizePolicy(sizePolicy);
	menu = new EntityListMenu(m_globalInfo);
	setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
	setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
	setDragEnabled(true);
	setDragDropMode(QAbstractItemView::DragDropMode::InternalMove);
	setAutoScroll(true);
	
	connect(this, &QListWidget::customContextMenuRequested, this, &EntityList::showMenu);
	connect(&globalInfo.signalEmitter, &SignalEmitter::createEntitySignal, this, &EntityList::onEntityCreated);
	connect(&globalInfo.signalEmitter, &SignalEmitter::destroyEntitySignal, this, &EntityList::onEntityDestroyed);
	connect(&globalInfo.signalEmitter, &SignalEmitter::selectEntitySignal, this, &EntityList::onEntitySelected);
	connect(&globalInfo.signalEmitter, &SignalEmitter::changeEntityNameSignal, this, &EntityList::onEntityNameChanged);
	connect(&globalInfo.signalEmitter, &SignalEmitter::changeEntityPersistenceSignal, this, &EntityList::onEntityPersistenceChanged);
	connect(&globalInfo.signalEmitter, &SignalEmitter::toggleEntityVisibilitySignal, this, &EntityList::onEntityVisibilityToggled);
	connect(itemDelegate(), &QAbstractItemDelegate::closeEditor, this, &EntityList::onLineEditClose);
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

std::vector<EntityID> EntityList::getRowSortedSelectedEntityIDs() {
	std::vector<std::pair<EntityID, int>> selectedEntities;
	for (QListWidgetItem* selectedItem : selectedItems()) {
		selectedEntities.push_back({ static_cast<EntityListItem*>(selectedItem)->entityID, row(selectedItem) });
	}

	std::sort(selectedEntities.begin(), selectedEntities.end(), [](const std::pair<EntityID, int>& a, const std::pair<EntityID, int>& b) {
		return a.second < b.second;
		});

	std::vector<EntityID> sortedSelectedEntities(selectedEntities.size());
	for (size_t i = 0; i < selectedEntities.size(); i++) {
		sortedSelectedEntities[i] = selectedEntities[i].first;
	}

	return sortedSelectedEntities;
}

void EntityList::updateSelection() {
	blockSelectionSignal = true;
	for (int i = 0; i < count(); i++) {
		EntityListItem* entityListItem = static_cast<EntityListItem*>(item(i));
		if (m_globalInfo.currentEntityID == entityListItem->entityID) {
			entityListItem->setSelected(true);
			setCurrentItem(entityListItem);
		}
		else if (m_globalInfo.otherSelectedEntityIDs.find(entityListItem->entityID) != m_globalInfo.otherSelectedEntityIDs.end()) {
			entityListItem->setSelected(true);
		}
		else {
			entityListItem->setSelected(false);
		}
	}
	blockSelectionSignal = false;
}

void EntityList::resizeFont(int delta) {
	QFont newFont = font();
	if ((newFont.pointSize() + delta) > 0) {
		newFont.setPointSize(newFont.pointSize() + delta);
		setFont(newFont);
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

void EntityList::onEntityPersistenceChanged(EntityID entityID, bool isPersistent) {
	(void)entityID;
	(void)isPersistent;

	SaveTitleChanger::change(m_globalInfo.mainWindow);
}

void EntityList::onEntityVisibilityToggled(EntityID entityID, bool isVisible) {
	EntityListItem* item = findItemWithEntityID(entityID);
	QFont font = item->font();
	if (isVisible) {
		font.setItalic(false);
		item->setForeground(QBrush());
	}
	else {
		font.setItalic(true);
		item->setForeground(QBrush(QColor(120.0f, 120.0f, 120.0f)));
	}
	findItemWithEntityID(entityID)->setFont(font);
}

void EntityList::showMenu(const QPoint& pos) {
	if (!itemAt(pos)) {
		menu->renameEntityAction->setEnabled(false);
		menu->deleteEntityAction->setEnabled(false);
	}
	else {
		EntityListItem* entityListItem = static_cast<EntityListItem*>(itemAt(pos));
		m_globalInfo.currentEntityID = entityListItem->entityID;
		menu->renameEntityAction->setEnabled(true);
		menu->deleteEntityAction->setEnabled(true);
	}
	menu->popup(QCursor::pos());
}

void EntityList::dropEvent(QDropEvent* event) {
	if (event->source() == this) {
		SaveTitleChanger::change(m_globalInfo.mainWindow);
	}
	QListWidget::dropEvent(event);
}

void EntityList::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected) {
	QListWidget::selectionChanged(selected, deselected);

	if (blockSelectionSignal) {
		return;
	}

	if (!selectedItems().empty()) {
		QList<QListWidgetItem*> otherSelectedItems = selectedItems();
		EntityID currentEntityID = static_cast<EntityListItem*>(otherSelectedItems.back())->entityID;
		otherSelectedItems.pop_back();

		std::set<EntityID> otherSelectedEntityIDs;
		for (QListWidgetItem* item : otherSelectedItems) {
			EntityListItem* entityListItem = static_cast<EntityListItem*>(item);

			otherSelectedEntityIDs.insert(entityListItem->entityID);
		}

		m_globalInfo.selectionUndoStack->push(new SelectAssetEntitiesCommand(m_globalInfo, SelectionType::Entities, "", currentEntityID, otherSelectedEntityIDs));
	}
	else {
		m_globalInfo.selectionUndoStack->push(new SelectAssetEntitiesCommand(m_globalInfo, SelectionType::Entities, "", NO_ENTITY, {}));
	}
}

void EntityList::keyPressEvent(QKeyEvent* event) {
	if (m_globalInfo.currentEntityID != NO_ENTITY) {
		EntityID currentEntityID = m_globalInfo.currentEntityID;
		std::set<EntityID> otherSelectedEntityIDs = m_globalInfo.otherSelectedEntityIDs;
		int currentSelectionIndex = row(findItemWithEntityID(m_globalInfo.currentEntityID));
		if (event->key() == Qt::Key_Delete) {
			std::vector<EntityID> entitiesToDestroy = { m_globalInfo.currentEntityID };
			std::copy(m_globalInfo.otherSelectedEntityIDs.begin(), m_globalInfo.otherSelectedEntityIDs.end(), std::back_inserter(entitiesToDestroy));
			m_globalInfo.actionUndoStack->push(new DestroyEntitiesCommand(m_globalInfo, entitiesToDestroy));
			if (count() != 0) {
				if (currentSelectionIndex < count()) {
					EntityListItem* entityListItem = static_cast<EntityListItem*>(item(currentSelectionIndex));
					currentEntityID = entityListItem->entityID;
				}
				else if (currentSelectionIndex > count()) {
					EntityListItem* entityListItem = static_cast<EntityListItem*>(item(count() - 1));
					currentEntityID = entityListItem->entityID;
				}
				else {
					EntityListItem* entityListItem = static_cast<EntityListItem*>(item(currentSelectionIndex - 1));
					currentEntityID = entityListItem->entityID;
				}
			}
		}
		else if (event->key() == Qt::Key_Up) {
			if (!m_moveEntityOrderKeyPressed) {
				currentEntityID = NO_ENTITY;
				otherSelectedEntityIDs.clear();
				if (currentSelectionIndex == 0) {
					EntityListItem* entityListItem = static_cast<EntityListItem*>(item(count() - 1));
					currentEntityID = entityListItem->entityID;
				}
				else {
					EntityListItem* entityListItem = static_cast<EntityListItem*>(item(currentSelectionIndex - 1));
					currentEntityID = entityListItem->entityID;
				}
			}
			else {
				std::vector<int> entityIndexes = { currentSelectionIndex };
				for (EntityID otherSelectedEntityID : otherSelectedEntityIDs) {
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

				SaveTitleChanger::change(m_globalInfo.mainWindow);
			}
		}
		else if (event->key() == Qt::Key_Down) {
			if (!m_moveEntityOrderKeyPressed) {
				currentEntityID = NO_ENTITY;
				otherSelectedEntityIDs.clear();
				if (currentSelectionIndex == (count() - 1)) {
					EntityListItem* entityListItem = static_cast<EntityListItem*>(item(0));
					currentEntityID = entityListItem->entityID;
				}
				else {
					EntityListItem* entityListItem = static_cast<EntityListItem*>(item(currentSelectionIndex + 1));
					currentEntityID = entityListItem->entityID;
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

				SaveTitleChanger::change(m_globalInfo.mainWindow);
			}
		}
		else if (event->key() == Qt::Key_Alt) {
			m_moveEntityOrderKeyPressed = true;
		}
		else if (event->key() == Qt::Key_F2) {
			EntityListItem* entityListItem = static_cast<EntityListItem*>(item(currentSelectionIndex));
			entityListItem->setFlags(entityListItem->flags() | Qt::ItemFlag::ItemIsEditable);
			editItem(entityListItem);
		}
		m_globalInfo.selectionUndoStack->push(new SelectAssetEntitiesCommand(m_globalInfo, SelectionType::Entities, "", currentEntityID, otherSelectedEntityIDs));
	}
}

void EntityList::keyReleaseEvent(QKeyEvent* event) {
	if (event->isAutoRepeat()) {
		event->accept();
		return;
	}

	if (event->key() == Qt::Key_Alt) {
		m_moveEntityOrderKeyPressed = false;
	}
}

void EntityList::wheelEvent(QWheelEvent* event) {
	if (QGuiApplication::keyboardModifiers() == Qt::ControlModifier) {
		int delta = event->angleDelta().y() / 120;
		resizeFont(delta);
		m_globalInfo.mainWindow->resourceSplitter->assetPanel->assetList->resizeFont(delta);
		m_globalInfo.mainWindow->resourceSplitter->scriptPanel->scriptList->resizeFont(delta);
	}
	else {
		QListWidget::wheelEvent(event);
	}
}

void EntityList::onLineEditClose(QWidget* lineEdit, QAbstractItemDelegate::EndEditHint hint) {
	(void)hint;
	EntityListItem* currentItem = findItemWithEntityID(m_globalInfo.currentEntityID);
	currentItem->setFlags(currentItem->flags() & ~Qt::ItemFlag::ItemIsEditable);
	std::string previousEntityName = m_globalInfo.entities[m_globalInfo.currentEntityID].name;
	std::string newEntityName = reinterpret_cast<QLineEdit*>(lineEdit)->text().toStdString();
	if (newEntityName.empty()) {
		currentItem->setText(QString::fromStdString(previousEntityName));
		return;
	}

	if (newEntityName == previousEntityName) {
		return;
	}

	if (m_globalInfo.findEntityByName(newEntityName) != NO_ENTITY) {
		m_globalInfo.logger.addLog(LogLevel::Warning, m_globalInfo.localization.getString("log_entity_name_already_taken", { previousEntityName, newEntityName }));

		currentItem->setText(QString::fromStdString(previousEntityName));
		return;
	}

	std::vector<EntityID> entityIDsToRename = m_globalInfo.mainWindow->entityPanel->entityList->getRowSortedSelectedEntityIDs();
	if (!entityIDsToRename.empty()) {
		m_globalInfo.actionUndoStack->push(new ChangeEntitiesNameCommand(m_globalInfo, entityIDsToRename, newEntityName));
	}
}
