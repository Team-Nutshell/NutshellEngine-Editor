#include "component_title_widget.h"
#include "main_window.h"
#include "../common/save_title_changer.h"
#include "../undo_commands/remove_entity_component_command.h"
#include <QHBoxLayout>
#include <QLabel>

ComponentTitleWidget::ComponentTitleWidget(GlobalInfo& globalInfo, const std::string& name): m_globalInfo(globalInfo) {
	m_name = name;
	
	setLayout(new QHBoxLayout());
	layout()->setContentsMargins(0, 0, 0, 0);
	if (name != "Transform") {
		removeWidgetButton = new QPushButton("X");
		removeWidgetButton->setFixedWidth(20);
		layout()->addWidget(removeWidgetButton);
		
		connect(removeWidgetButton, &QPushButton::clicked, this, &ComponentTitleWidget::onClick);
	}
	layout()->addWidget(new QLabel("<b>" + QString::fromStdString(name) + "</b>"));
}

void ComponentTitleWidget::onClick() {
	m_globalInfo.undoStack->push(new RemoveEntityComponentCommand(m_globalInfo, m_globalInfo.currentEntityID, m_name));

	SaveTitleChanger::change(m_globalInfo.mainWindow);
}