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
	layout()->addWidget(new SeparatorLine());

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

void LightComponentWidget::updateComponents(const std::vector<EntityID>& entityIDs, std::vector<Light>& lights) {
	std::vector<Component*> componentPointers;
	for (size_t i = 0; i < lights.size(); i++) {
		componentPointers.push_back(&lights[i]);
	}

	m_globalInfo.actionUndoStack->push(new ChangeEntitiesComponentCommand(m_globalInfo, entityIDs, "Light", componentPointers));
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
	QObject* senderWidget = sender();

	std::vector<EntityID> entityIDs;
	std::vector<Light> newLights;

	std::set<EntityID> selectedEntityIDs = m_globalInfo.otherSelectedEntityIDs;
	selectedEntityIDs.insert(m_globalInfo.currentEntityID);
	for (EntityID selectedEntityID : selectedEntityIDs) {
		if (m_globalInfo.entities[selectedEntityID].light) {
			Light newLight = m_globalInfo.entities[selectedEntityID].light.value();

			if (senderWidget == typeWidget) {
				newLight.type = lightTypeToType(element);
			}

			entityIDs.push_back(selectedEntityID);
			newLights.push_back(newLight);
		}
	}

	updateComponents(entityIDs, newLights);
}

void LightComponentWidget::onColorChanged(const nml::vec3& color) {
	QObject* senderWidget = sender();

	std::vector<EntityID> entityIDs;
	std::vector<Light> newLights;

	std::set<EntityID> selectedEntityIDs = m_globalInfo.otherSelectedEntityIDs;
	selectedEntityIDs.insert(m_globalInfo.currentEntityID);
	for (EntityID selectedEntityID : selectedEntityIDs) {
		if (m_globalInfo.entities[selectedEntityID].light) {
			Light newLight = m_globalInfo.entities[selectedEntityID].light.value();

			if (senderWidget == colorWidget) {
				newLight.color = color;
			}

			entityIDs.push_back(selectedEntityID);
			newLights.push_back(newLight);
		}
	}

	updateComponents(entityIDs, newLights);
}

void LightComponentWidget::onScalarChanged(float value) {
	QObject* senderWidget = sender();

	std::vector<EntityID> entityIDs;
	std::vector<Light> newLights;

	std::set<EntityID> selectedEntityIDs = m_globalInfo.otherSelectedEntityIDs;
	selectedEntityIDs.insert(m_globalInfo.currentEntityID);
	for (EntityID selectedEntityID : selectedEntityIDs) {
		if (m_globalInfo.entities[selectedEntityID].light) {
			Light newLight = m_globalInfo.entities[selectedEntityID].light.value();

			if (senderWidget == intensityWidget) {
				newLight.intensity = value;
			}

			entityIDs.push_back(selectedEntityID);
			newLights.push_back(newLight);
		}
	}

	updateComponents(entityIDs, newLights);
}

void LightComponentWidget::onVec3Changed(const nml::vec3& value) {
	QObject* senderWidget = sender();

	std::vector<EntityID> entityIDs;
	std::vector<Light> newLights;

	Light newLight = m_globalInfo.entities[m_globalInfo.currentEntityID].light.value();

	uint8_t changedIndex = 255;
	if (senderWidget == directionWidget) {
		for (uint8_t i = 0; i < 2; i++) {
			if (newLight.direction[i] != value[i]) {
				changedIndex = i;
				break;
			}
		}
		newLight.direction = value;
	}
	entityIDs.push_back(m_globalInfo.currentEntityID);
	newLights.push_back(newLight);

	for (EntityID otherSelectedEntityID : m_globalInfo.otherSelectedEntityIDs) {
		if (m_globalInfo.entities[otherSelectedEntityID].light) {
			newLight = m_globalInfo.entities[otherSelectedEntityID].light.value();

			if (senderWidget == directionWidget) {
				newLight.direction[changedIndex] = value[changedIndex];
			}

			entityIDs.push_back(otherSelectedEntityID);
			newLights.push_back(newLight);
		}
	}

	updateComponents(entityIDs, newLights);
}

void LightComponentWidget::onVec2Changed(const nml::vec2& value) {
	QObject* senderWidget = sender();

	std::vector<EntityID> entityIDs;
	std::vector<Light> newLights;

	Light newLight = m_globalInfo.entities[m_globalInfo.currentEntityID].light.value();

	uint8_t changedIndex = 255;
	if (senderWidget == cutoffWidget) {
		for (uint8_t i = 0; i < 2; i++) {
			if (newLight.cutoff[i] != value[i]) {
				changedIndex = i;
				break;
			}
		}
		newLight.cutoff = value;
	}
	entityIDs.push_back(m_globalInfo.currentEntityID);
	newLights.push_back(newLight);

	for (EntityID otherSelectedEntityID : m_globalInfo.otherSelectedEntityIDs) {
		if (m_globalInfo.entities[otherSelectedEntityID].light) {
			newLight = m_globalInfo.entities[otherSelectedEntityID].light.value();

			if (senderWidget == cutoffWidget) {
				newLight.cutoff[changedIndex] = value[changedIndex];
			}

			entityIDs.push_back(otherSelectedEntityID);
			newLights.push_back(newLight);
		}
	}

	updateComponents(entityIDs, newLights);
}
