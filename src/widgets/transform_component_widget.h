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
	void updateComponents(const std::vector<EntityID>& entityIDs, std::vector<Transform>& transforms);

private slots:
	void onEntitySelected();
	void onEntityTransformChanged(EntityID entityID, const Transform& transform);
	void onXChanged(float value);
	void onYChanged(float value);
	void onZChanged(float value);

private:
	GlobalInfo& m_globalInfo;

public:
	Vector3Widget* positionWidget;
	Vector3Widget* rotationWidget;
	Vector3Widget* scaleWidget;
};