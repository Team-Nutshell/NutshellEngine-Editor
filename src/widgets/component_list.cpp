#include "component_list.h"
#include <QVBoxLayout>

ComponentList::ComponentList(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setLayout(new QVBoxLayout());
	layout()->setAlignment(Qt::AlignmentFlag::AlignTop);
	layout()->setContentsMargins(0, 0, 0, 0);
	transformWidget = std::make_unique<TransformComponentWidget>(m_globalInfo);
	layout()->addWidget(transformWidget.get());
	cameraWidget = std::make_unique<CameraComponentWidget>(m_globalInfo);
	layout()->addWidget(cameraWidget.get());
	lightWidget = std::make_unique<LightComponentWidget>(m_globalInfo);
	layout()->addWidget(lightWidget.get());
	renderableWidget = std::make_unique<RenderableComponentWidget>(m_globalInfo);
	layout()->addWidget(renderableWidget.get());
	rigidbodyWidget = std::make_unique<RigidbodyComponentWidget>(m_globalInfo);
	layout()->addWidget(rigidbodyWidget.get());
	collidableWidget = std::make_unique<CollidableComponentWidget>(m_globalInfo);
	layout()->addWidget(collidableWidget.get());
	scriptableWidget = std::make_unique<ScriptableComponentWidget>(m_globalInfo);
	layout()->addWidget(scriptableWidget.get());
	addComponentButton = std::make_unique<AddComponentButton>(m_globalInfo);
	layout()->addWidget(addComponentButton.get());
}
