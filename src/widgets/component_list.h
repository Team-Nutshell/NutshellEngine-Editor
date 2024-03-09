#pragma once
#include "../common/common.h"
#include "transform_component_widget.h"
#include "camera_component_widget.h"
#include "light_component_widget.h"
#include "renderable_component_widget.h"
#include "rigidbody_component_widget.h"
#include "collidable_component_widget.h"
#include "scriptable_component_widget.h"
#include "add_component_button.h"
#include <QWidget>
#include <memory>

class ComponentList : public QWidget {
	Q_OBJECT
public:
	ComponentList(GlobalInfo& globalInfo);

private:
	GlobalInfo& m_globalInfo;

	std::unique_ptr<TransformComponentWidget> transformWidget;
	std::unique_ptr<CameraComponentWidget> cameraWidget;
	std::unique_ptr<LightComponentWidget> lightWidget;
	std::unique_ptr<RenderableComponentWidget> renderableWidget;
	std::unique_ptr<RigidbodyComponentWidget> rigidbodyWidget;
	std::unique_ptr<CollidableComponentWidget> collidableWidget;
	std::unique_ptr<ScriptableComponentWidget> scriptableWidget;
	std::unique_ptr<AddComponentButton> addComponentButton;
};