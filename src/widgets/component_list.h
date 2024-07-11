#pragma once
#include "../common/global_info.h"
#include "transform_component_widget.h"
#include "camera_component_widget.h"
#include "light_component_widget.h"
#include "renderable_component_widget.h"
#include "rigidbody_component_widget.h"
#include "collidable_component_widget.h"
#include "scriptable_component_widget.h"
#include "add_component_button.h"
#include <QWidget>

class ComponentList : public QWidget {
	Q_OBJECT
public:
	ComponentList(GlobalInfo& globalInfo);

private:
	GlobalInfo& m_globalInfo;

	TransformComponentWidget* transformWidget;
	CameraComponentWidget* cameraWidget;
	LightComponentWidget* lightWidget;
	RenderableComponentWidget* renderableWidget;
	RigidbodyComponentWidget* rigidbodyWidget;
	CollidableComponentWidget* collidableWidget;
	ScriptableComponentWidget* scriptableWidget;
	AddComponentButton* addComponentButton;
};