#pragma once
#include "../common/global_info.h"
#include "combo_box_widget.h"
#include "boolean_widget.h"
#include "vector3_widget.h"
#include "scalar_widget.h"
#include <QWidget>

class CollidableComponentWidget : public QWidget {
	Q_OBJECT
public:
	CollidableComponentWidget(GlobalInfo& globalInfo);

private:
	void updateWidgets(const Collidable& collidable);

private slots:
	void onSelectEntity();
	void onAddEntityCollidable(EntityID entityID);
	void onRemoveEntityCollidable(EntityID entityID);
	void onChangeEntityCollidable(EntityID entityID, const Collidable& collidable);
	void onElementUpdated(const std::string& element);
	void onVec3Updated(const nml::vec3& value);
	void onBooleanUpdated(bool boolean);
	void onScalarUpdated(float value);

private:
	GlobalInfo& m_globalInfo;

public:
	ComboBoxWidget* typeWidget;
	BooleanWidget* fromRenderableWidget;
	Vector3Widget* centerWidget;
	ScalarWidget* radiusWidget;
	Vector3Widget* halfExtentWidget;
	Vector3Widget* rotationWidget;
	Vector3Widget* baseWidget;
	Vector3Widget* tipWidget;
};