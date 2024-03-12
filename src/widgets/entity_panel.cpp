#include "entity_panel.h"
#include <QVBoxLayout>
#include <QLabel>

EntityPanel::EntityPanel(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	resize(100, height());
	setLayout(new QVBoxLayout());
	layout()->setContentsMargins(2, 2, 0, 2);
	layout()->addWidget(new QLabel("Entity List"));
	entityList = new EntityList(m_globalInfo);
	layout()->addWidget(entityList);
}