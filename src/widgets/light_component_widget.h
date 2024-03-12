#pragma once
#include "../common/common.h"
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

private slots:
	void onSelectEntity();
	void onAddEntityLight(EntityID entityID);
	void onRemoveEntityLight(EntityID entityID);
	void onChangeEntityLight(EntityID entityID, const Light& light);
	void onElementUpdated(const std::string& element);
	void onColorUpdated(const nml::vec4& color);
	void onVec3Updated(const nml::vec3& value);
	void onVec2Updated(const nml::vec2& value);

private:
	GlobalInfo& m_globalInfo;

public:
	ComboBoxWidget* typeWidget;
	ColorPickerWidget* colorWidget;
	Vector3Widget* directionWidget;
	Vector2Widget* cutoffWidget;
};