#pragma once
#include "../common/common.h"
#include "vector3_widget.h"
#include <QWidget>

class TransformComponentWidget : public QWidget {
	Q_OBJECT
public:
	TransformComponentWidget(GlobalInfo& globalInfo);

private:
	void updateWidgets(const Transform& transform);

private slots:
	void onSelectEntity();
	void onChangeEntityTransform(EntityID entityID, const Transform& transform);
	void onVec3Updated(const nml::vec3& value);

private:
	GlobalInfo& m_globalInfo;

public:
	Vector3Widget* positionWidget;
	Vector3Widget* rotationWidget;
	Vector3Widget* scaleWidget;
};