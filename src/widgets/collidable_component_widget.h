#pragma once
#include "../common/common.h"
#include "combo_box_widget.h"
#include "boolean_widget.h"
#include "vector3_widget.h"
#include "scalar_widget.h"
#include <QWidget>
#include <memory>

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
	std::unique_ptr<ComboBoxWidget> typeWidget;
	std::unique_ptr<BooleanWidget> fromRenderableWidget;
	std::unique_ptr<Vector3Widget> centerWidget;
	std::unique_ptr<ScalarWidget> radiusWidget;
	std::unique_ptr<Vector3Widget> halfExtentWidget;
	std::unique_ptr<Vector3Widget> rotationWidget;
	std::unique_ptr<Vector3Widget> baseWidget;
	std::unique_ptr<Vector3Widget> tipWidget;
};