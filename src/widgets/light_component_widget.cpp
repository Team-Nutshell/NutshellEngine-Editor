#include "light_component_widget.h"
#include "component_title_widget.h"
#include "separator_line.h"
#include "main_window.h"
#include "../common/save_title_changer.h"
#include "../undo_commands/change_entities_component_command.h"
#include <QVBoxLayout>
#include <QSignalBlocker>
#include <vector>
#include <string>

LightComponentWidget::LightComponentWidget(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setLayout(new QVBoxLayout());
	layout()->setAlignment(Qt::AlignmentFlag::AlignTop);
	layout()->setContentsMargins(0, 0, 0, 0);
	layout()->addWidget(new ComponentTitleWidget(m_globalInfo, m_globalInfo.localization.getString("component_light")));
	std::vector<std::string> elementList = { m_globalInfo.localization.getString("component_light_type_directional"), m_globalInfo.localization.getString("component_light_type_point"), m_globalInfo.localization.getString("component_light_type_spot"), m_globalInfo.localization.getString("component_light_type_ambient") };
	typeWidget = new ComboBoxWidget(m_globalInfo, m_globalInfo.localization.getString("component_light_type"), elementList);
	layout()->addWidget(typeWidget);
	colorWidget = new ColorPickerWidget(m_globalInfo, m_globalInfo.localization.getString("component_light_color"), nml::vec3(1.0f, 1.0f, 1.0f));
	layout()->addWidget(colorWidget);
	intensityWidget = new ScalarWidget(m_globalInfo, m_globalInfo.localization.getString("component_light_intensity"));
	layout()->addWidget(intensityWidget);
	directionWidget = new Vector3Widget(m_globalInfo, m_globalInfo.localization.getString("component_light_direction"));
	layout()->addWidget(directionWidget);
	cutoffWidget = new Vector2Widget(m_globalInfo, m_globalInfo.localization.getString("component_light_cutoff"));
	layout()->addWidget(cutoffWidget);
	layout()->addWidget(new SeparatorLine(m_globalInfo));

	connect(typeWidget, &ComboBoxWidget::elementSelected, this, &LightComponentWidget::onElementChanged);
	connect(colorWidget, &ColorPickerWidget::colorChanged, this, &LightComponentWidget::onColorChanged);
	connect(intensityWidget, &ScalarWidget::valueChanged, this, &LightComponentWidget::onScalarChanged);
	connect(directionWidget, &Vector3Widget::valueChanged, this, &LightComponentWidget::onVec3Changed);
	connect(cutoffWidget, &Vector2Widget::valueChanged, this, &LightComponentWidget::onVec2Changed);
	connect(&globalInfo.signalEmitter, &SignalEmitter::selectEntitySignal, this, &LightComponentWidget::onEntitySelected);
	connect(&globalInfo.signalEmitter, &SignalEmitter::addEntityLightSignal, this, &LightComponentWidget::onEntityLightAdded);
	connect(&globalInfo.signalEmitter, &SignalEmitter::removeEntityLightSignal, this, &LightComponentWidget::onEntityLightRemoved);
	connect(&globalInfo.signalEmitter, &SignalEmitter::changeEntityLightSignal, this, &LightComponentWidget::onEntityLightChanged);
}

void LightComponentWidget::updateWidgets(const Light& light) {
	typeWidget->setElementByText(typeToLightType(light.type));
	colorWidget->setColor(nml::vec4(light.color, 1.0f));
	intensityWidget->setValue(light.intensity);
	directionWidget->setValue(light.direction);
	if ((light.type == "Directional") || (light.type == "Spot")) {
		directionWidget->setEnabled(true);
	}
	else {
		directionWidget->setEnabled(false);
	}
	cutoffWidget->setValue(light.cutoff);
	if (light.type == "Spot") {
		cutoffWidget->setEnabled(true);
	}
	else {
		cutoffWidget->setEnabled(false);
	}
}

void LightComponentWidget::updateComponent(EntityID entityID, Component* component) {
	m_globalInfo.undoStack->push(new ChangeEntitiesComponentCommand(m_globalInfo, { entityID }, "Light", { component }));
}

std::string LightComponentWidget::lightTypeToType(const std::string& lightType) {
	if (lightType == m_globalInfo.localization.getString("component_light_type_directional")) {
		return "Directional";
	}
	else if (lightType == m_globalInfo.localization.getString("component_light_type_point")) {
		return "Point";
	}
	else if (lightType == m_globalInfo.localization.getString("component_light_type_spot")) {
		return "Spot";
	}
	else if (lightType == m_globalInfo.localization.getString("component_light_type_ambient")) {
		return "Ambient";
	}
	else {
		return "Unknown";
	}
}

std::string LightComponentWidget::typeToLightType(const std::string& type) {
	if (type == "Directional") {
		return m_globalInfo.localization.getString("component_light_type_directional");
	}
	else if (type == "Point") {
		return m_globalInfo.localization.getString("component_light_type_point");
	}
	else if (type == "Spot") {
		return m_globalInfo.localization.getString("component_light_type_spot");
	}
	else if (type == "Ambient") {
		return m_globalInfo.localization.getString("component_light_type_ambient");
	}
	else {
		return "Unknown";
	}
}

void LightComponentWidget::onEntitySelected() {
	if ((m_globalInfo.currentEntityID != NO_ENTITY) && m_globalInfo.entities[m_globalInfo.currentEntityID].light.has_value()) {
		show();
		updateWidgets(m_globalInfo.entities[m_globalInfo.currentEntityID].light.value());
	}
	else {
		hide();
	}
}

void LightComponentWidget::onEntityLightAdded(EntityID entityID) {
	if (entityID == m_globalInfo.currentEntityID) {
		const Light& light = m_globalInfo.entities[m_globalInfo.currentEntityID].light.value();
		updateWidgets(light);
		show();
	}

	SaveTitleChanger::change(m_globalInfo.mainWindow);
}

void LightComponentWidget::onEntityLightRemoved(EntityID entityID) {
	if (entityID == m_globalInfo.currentEntityID) {
		hide();
	}

	SaveTitleChanger::change(m_globalInfo.mainWindow);
}

void LightComponentWidget::onEntityLightChanged(EntityID entityID, const Light& light) {
	QObject* senderWidget = sender();
	if (senderWidget != this) {
		if (entityID == m_globalInfo.currentEntityID) {
			updateWidgets(light);
		}
	}

	SaveTitleChanger::change(m_globalInfo.mainWindow);
}

void LightComponentWidget::onElementChanged(const std::string& element) {
	Light newLight = m_globalInfo.entities[m_globalInfo.currentEntityID].light.value();

	QObject* senderWidget = sender();
	if (senderWidget == typeWidget) {
		newLight.type = lightTypeToType(element);
	}
	updateComponent(m_globalInfo.currentEntityID, &newLight);
}

void LightComponentWidget::onColorChanged(const nml::vec3& color) {
	Light newLight = m_globalInfo.entities[m_globalInfo.currentEntityID].light.value();

	QObject* senderWidget = sender();
	if (senderWidget == colorWidget) {
		newLight.color = color;
	}
	updateComponent(m_globalInfo.currentEntityID, &newLight);
}

void LightComponentWidget::onScalarChanged(float value) {
	Light newLight = m_globalInfo.entities[m_globalInfo.currentEntityID].light.value();

	QObject* senderWidget = sender();
	if (senderWidget == intensityWidget) {
		newLight.intensity = value;
	}
	updateComponent(m_globalInfo.currentEntityID, &newLight);
}

void LightComponentWidget::onVec3Changed(const nml::vec3& value) {
	Light newLight = m_globalInfo.entities[m_globalInfo.currentEntityID].light.value();

	QObject* senderWidget = sender();
	if (senderWidget == directionWidget) {
		newLight.direction = value;
	}
	updateComponent(m_globalInfo.currentEntityID, &newLight);
}

void LightComponentWidget::onVec2Changed(const nml::vec2& value) {
	Light newLight = m_globalInfo.entities[m_globalInfo.currentEntityID].light.value();

	QObject* senderWidget = sender();
	if (senderWidget == cutoffWidget) {
		newLight.cutoff = value;
	}
	updateComponent(m_globalInfo.currentEntityID, &newLight);
}
