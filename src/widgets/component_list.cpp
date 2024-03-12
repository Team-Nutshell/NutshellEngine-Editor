#include "component_list.h"
#include <QVBoxLayout>

ComponentList::ComponentList(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setLayout(new QVBoxLayout());
	layout()->setAlignment(Qt::AlignmentFlag::AlignTop);
	layout()->setContentsMargins(0, 0, 0, 0);
	transformWidget = new TransformComponentWidget(m_globalInfo);
	layout()->addWidget(transformWidget);
	cameraWidget = new CameraComponentWidget(m_globalInfo);
	layout()->addWidget(cameraWidget);
	lightWidget = new LightComponentWidget(m_globalInfo);
	layout()->addWidget(lightWidget);
	renderableWidget = new RenderableComponentWidget(m_globalInfo);
	layout()->addWidget(renderableWidget);
	rigidbodyWidget = new RigidbodyComponentWidget(m_globalInfo);
	layout()->addWidget(rigidbodyWidget);
	collidableWidget = new CollidableComponentWidget(m_globalInfo);
	layout()->addWidget(collidableWidget);
	scriptableWidget = new ScriptableComponentWidget(m_globalInfo);
	layout()->addWidget(scriptableWidget);
	addComponentButton = new AddComponentButton(m_globalInfo);
	layout()->addWidget(addComponentButton);
}
