#pragma once
#include "../common/global_info.h"
#include "combo_box_widget.h"
#include "vector3_widget.h"
#include "scalar_widget.h"
#include <QWidget>
#include <QPushButton>

class CollidableComponentWidget : public QWidget {
	Q_OBJECT
public:
	CollidableComponentWidget(GlobalInfo& globalInfo);

private:
	void updateWidgets(const Collidable& collidable);
	void updateComponent(EntityID entityID, Component* component);

	void updateFromRenderableWidget();

	std::string colliderTypeToType(const std::string& colliderType);
	std::string typeToColliderType(const std::string& type);

private slots:
	void onEntityCreated(EntityID entityID);
	void onEntitySelected();
	void onEntityCollidableAdded(EntityID entityID);
	void onEntityCollidableRemoved(EntityID entityID);
	void onEntityCollidableChanged(EntityID entityID, const Collidable& collidable);
	void onElementChanged(const std::string& element);
	void onVec3Changed(const nml::vec3& value);
	void onScalarChanged(float value);
	void onFromRenderableButtonClicked();
	void onEntityRenderableAdded(EntityID entityID);
	void onEntityRenderableRemoved(EntityID entityID);
	void onEntityRenderableChanged(EntityID entityID);

private:
	GlobalInfo& m_globalInfo;

public:
	ComboBoxWidget* typeWidget;
	Vector3Widget* centerWidget;
	ScalarWidget* radiusWidget;
	Vector3Widget* halfExtentWidget;
	Vector3Widget* rotationWidget;
	Vector3Widget* baseWidget;
	Vector3Widget* tipWidget;
	QPushButton* fromRenderableWidget;
};