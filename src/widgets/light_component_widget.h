#pragma once
#include "../common/global_info.h"
#include "combo_box_widget.h"
#include "color_picker_widget.h"
#include "vector3_widget.h"
#include "vector2_widget.h"
#include <QWidget>

class LightComponentWidget : public QWidget {
	Q_OBJECT
public:
	LightComponentWidget(GlobalInfo& globalInfo);

private:
	void updateWidgets(const Light& light);
	void updateComponent(EntityID entityID, Component* component);

private slots:
	void onEntitySelected();
	void onAddEntityLight(EntityID entityID);
	void onRemoveEntityLight(EntityID entityID);
	void onChangeEntityLight(EntityID entityID, const Light& light);
	void onElementChanged(const std::string& element);
	void onColorChanged(const nml::vec4& color);
	void onVec3Changed(const nml::vec3& value);
	void onVec2Changed(const nml::vec2& value);

private:
	GlobalInfo& m_globalInfo;

public:
	ComboBoxWidget* typeWidget;
	ColorPickerWidget* colorWidget;
	Vector3Widget* directionWidget;
	Vector2Widget* cutoffWidget;
};