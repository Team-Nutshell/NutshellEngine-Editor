#include "component_title_widget.h"
#include "main_window.h"
#include "../common/save_title_changer.h"
#include "../undo_commands/remove_entity_component_command.h"
#include <QHBoxLayout>
#include <QLabel>

ComponentTitleWidget::ComponentTitleWidget(GlobalInfo& globalInfo, const std::string& name) : m_globalInfo(globalInfo) {
	m_name = name;
	
	setLayout(new QHBoxLayout());
	layout()->setContentsMargins(0, 0, 0, 0);
	if (name != "Transform") {
		removeWidgetButton = new QPushButton("X");
		removeWidgetButton->setFixedWidth(20);
		layout()->addWidget(removeWidgetButton);
		
		connect(removeWidgetButton, &QPushButton::clicked, this, &ComponentTitleWidget::onRemoveWidgetClicked);
	}
	layout()->addWidget(new QLabel("<b>" + QString::fromStdString(name) + "</b>"));
}

void ComponentTitleWidget::onRemoveWidgetClicked() {
	std::set<EntityID> selectedEntityIDs = m_globalInfo.otherSelectedEntityIDs;
	selectedEntityIDs.insert(m_globalInfo.currentEntityID);

	std::vector<EntityID> entityIDs;
	for (EntityID selectedEntityID : selectedEntityIDs) {
		if (((m_name == "Camera") && m_globalInfo.entities[selectedEntityID].camera) ||
			((m_name == "Light") && m_globalInfo.entities[selectedEntityID].light) ||
			((m_name == "Renderable") && m_globalInfo.entities[selectedEntityID].renderable) ||
			((m_name == "Rigidbody") && m_globalInfo.entities[selectedEntityID].rigidbody) ||
			((m_name == "Collidable") && m_globalInfo.entities[selectedEntityID].collidable) ||
			((m_name == "SoundListener") && m_globalInfo.entities[selectedEntityID].soundListener) ||
			((m_name == "Scriptable") && m_globalInfo.entities[selectedEntityID].scriptable)) {
			entityIDs.push_back(selectedEntityID);
		}
	}

	m_globalInfo.actionUndoStack->push(new RemoveEntityComponentCommand(m_globalInfo, entityIDs, m_name));

	SaveTitleChanger::change(m_globalInfo.mainWindow);
}