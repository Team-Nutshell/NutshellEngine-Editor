#pragma once
#include "../common/global_info.h"
#include "vector3_widget.h"
#include <QWidget>

class TransformComponentWidget : public QWidget {
	Q_OBJECT
public:
	TransformComponentWidget(GlobalInfo& globalInfo);

	void updateWidgets(const Transform& transform);

private:
	void updateComponent(EntityID entityID, Component* component);

private slots:
	void onEntitySelected();
	void onEntityTransformChanged(EntityID entityID, const Transform& transform);
	void onVec3Changed(const nml::vec3& value);

private:
	GlobalInfo& m_globalInfo;

public:
	Vector3Widget* positionWidget;
	Vector3Widget* rotationWidget;
	Vector3Widget* scaleWidget;
};