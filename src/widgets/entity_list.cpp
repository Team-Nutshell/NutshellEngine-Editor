#include "entity_list.h"
#include "../undo_commands/destroy_entity_command.h"
#include <QSizePolicy>
#include <QLabel>
#include <QSignalBlocker>
#include <QFont>

EntityList::EntityList(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	QSizePolicy sizePolicy;
	sizePolicy.setHorizontalPolicy(QSizePolicy::Policy::Ignored);
	sizePolicy.setVerticalPolicy(QSizePolicy::Policy::Expanding);
	setSizePolicy(sizePolicy);
	menu = new EntityListMenu(m_globalInfo);
	setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
	
	connect(this, &QListWidget::customContextMenuRequested, this, &EntityList::showMenu);
	connect(this, &QListWidget::itemSelectionChanged, this, &EntityList::onItemSelectionChanged);
	connect(&globalInfo.signalEmitter, &SignalEmitter::createEntitySignal, this, &EntityList::onCreateEntity);
	connect(&globalInfo.signalEmitter, &SignalEmitter::destroyEntitySignal, this, &EntityList::onDestroyEntity);
	connect(&globalInfo.signalEmitter, &SignalEmitter::selectEntitySignal, this, &EntityList::onSelectEntity);
	connect(&globalInfo.signalEmitter, &SignalEmitter::changeEntityNameSignal, this, &EntityList::onChangeEntityName);
	connect(&globalInfo.signalEmitter, &SignalEmitter::toggleCurrentEntityVisibilitySignal, this, &EntityList::onToggleCurrentEntityVisibility);
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

void EntityList::onCreateEntity(EntityID entityID) {
	addItem(new EntityListItem(m_globalInfo, entityID));
}

void EntityList::onDestroyEntity(EntityID entityID) {
	takeItem(row(findItemWithEntityID(entityID)));
}

void EntityList::onSelectEntity() {
	{
		const QSignalBlocker signalBlocker(this);
		clearSelection();
	}

	if (m_globalInfo.currentEntityID != NO_ENTITY) {
		{
			const QSignalBlocker signalBlocker(this);
			findItemWithEntityID(m_globalInfo.currentEntityID)->setSelected(true);
		}
	}
}

void EntityList::onChangeEntityName(EntityID entityID, const std::string& name) {
	findItemWithEntityID(entityID)->setText(QString::fromStdString(name));
}

void EntityList::onToggleCurrentEntityVisibility(bool isVisible) {
	QFont font = findItemWithEntityID(m_globalInfo.currentEntityID)->font();
	if (isVisible) {
		font.setItalic(false);
	}
	else {
		font.setItalic(true);
	}
	findItemWithEntityID(m_globalInfo.currentEntityID)->setFont(font);
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

void EntityList::onItemSelectionChanged() {
	if (!selectedItems().empty()) {
		EntityListItem* entityListItem = static_cast<EntityListItem*>(selectedItems()[0]);
		m_globalInfo.currentEntityID = entityListItem->entityID;
	}
	else {
		m_globalInfo.currentEntityID = NO_ENTITY;
	}
	emit m_globalInfo.signalEmitter.selectEntitySignal();
}

void EntityList::keyPressEvent(QKeyEvent* event) {
	if (event->isAutoRepeat()) {
		event->accept();
		return;
	}

	if (!selectedItems().empty()) {
		EntityListItem* entityListItem = static_cast<EntityListItem*>(selectedItems()[0]);
		int currentSelectionIndex = row(entityListItem);
		if (event->key() == Qt::Key::Key_Delete) {
			m_globalInfo.undoStack->push(new DestroyEntityCommand(m_globalInfo, entityListItem->entityID));
		}
		else if (event->key() == Qt::Key::Key_Up) {
			clearSelection();
			if (currentSelectionIndex == 0) {
				setCurrentItem(item(count() - 1));
			}
			else {
				setCurrentItem(item(currentSelectionIndex - 1));
			}
		}
		else if (event->key() == Qt::Key::Key_Down) {
			clearSelection();
			if (currentSelectionIndex == (count() - 1)) {
				setCurrentItem(item(0));
			}
			else {
				setCurrentItem(item(currentSelectionIndex + 1));
			}
		}
	}
}