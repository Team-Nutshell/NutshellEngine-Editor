#pragma once
#include "../common/common.h"
#include "vector3_widget.h"
#include <QWidget>
#include <memory>

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
	std::unique_ptr<Vector3Widget> positionWidget;
	std::unique_ptr<Vector3Widget> rotationWidget;
	std::unique_ptr<Vector3Widget> scaleWidget;
};