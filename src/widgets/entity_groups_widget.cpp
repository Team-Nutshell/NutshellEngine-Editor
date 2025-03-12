#include "entity_groups_widget.h"
#include "main_window.h"
#include "../common/save_title_changer.h"
#include "../undo_commands/change_entities_entity_groups_command.h"
#include <QVBoxLayout>

EntityGroupsWidget::EntityGroupsWidget(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setLayout(new QVBoxLayout());
	layout()->setContentsMargins(0, 0, 0, 0);
	addEntityGroupButton = new QPushButton("+");
	layout()->addWidget(addEntityGroupButton);

	connect(&globalInfo.signalEmitter, &SignalEmitter::selectEntitySignal, this, &EntityGroupsWidget::onEntitySelected);
	connect(addEntityGroupButton, &QPushButton::clicked, this, &EntityGroupsWidget::onAddEntityGroupClicked);
}

void EntityGroupsWidget::updateEntityGroups() {
	std::vector<EntityID> changedEntityIDs = { m_globalInfo.currentEntityID };
	for (EntityID otherSelectedEntityID : m_globalInfo.otherSelectedEntityIDs) {
		changedEntityIDs.push_back(otherSelectedEntityID);
	}
	std::vector<std::string> changedEntityGroupNames;
	for (int i = 0; i < layout()->count() - 1; i++) {
		std::string entityGroupName = static_cast<EntityGroupNameWidget*>(layout()->itemAt(i)->widget())->getText();
		if (!entityGroupName.empty()) {
			changedEntityGroupNames.push_back(entityGroupName);
		}
	}
	m_globalInfo.actionUndoStack->push(new ChangeEntitiesEntityGroupsCommand(m_globalInfo, changedEntityIDs, changedEntityGroupNames));

	emit updateEntityGroupsSignal();
}

EntityGroupNameWidget* EntityGroupsWidget::addEntityGroupNameWidget() {
	EntityGroupNameWidget* newEntityGroupNameWidget = new EntityGroupNameWidget(m_globalInfo);
	static_cast<QVBoxLayout*>(layout())->insertWidget(layout()->count() - 1, newEntityGroupNameWidget);

	connect(newEntityGroupNameWidget, &EntityGroupNameWidget::changeEntityGroupNameSignal, this, &EntityGroupsWidget::onEntityGroupNameChanged);
	connect(newEntityGroupNameWidget, &EntityGroupNameWidget::removeEntityGroupSignal, this, &EntityGroupsWidget::onRemoveEntityGroupClicked);

	return newEntityGroupNameWidget;
}

void EntityGroupsWidget::onEntitySelected() {
	if (m_globalInfo.currentEntityID != NO_ENTITY) {
		QLayoutItem* item = nullptr;
		while ((layout()->count() != 1) && (item = layout()->takeAt(0))) {
			delete item->widget();
			delete item;
		}

		for (const std::string& entityGroupName : m_globalInfo.entities[m_globalInfo.currentEntityID].entityGroups) {
			EntityGroupNameWidget* newEntityGroupNameWidget = addEntityGroupNameWidget();
			newEntityGroupNameWidget->setText(entityGroupName);
		}
	}
}

void EntityGroupsWidget::onEntityGroupNameChanged() {
	updateEntityGroups();

	SaveTitleChanger::change(m_globalInfo.mainWindow);
}

void EntityGroupsWidget::onRemoveEntityGroupClicked() {
	for (int i = 0; i < layout()->count() - 1; i++) {
		if (layout()->itemAt(i)->widget() == sender()) {
			QLayoutItem* item = layout()->takeAt(i);
			delete item->widget();
			delete item;

			break;
		}
	}
	updateEntityGroups();

	SaveTitleChanger::change(m_globalInfo.mainWindow);
}

void EntityGroupsWidget::onAddEntityGroupClicked() {
	addEntityGroupNameWidget();
}