#include "light_component_widget.h"
#include "component_title_widget.h"
#include "separator_line.h"
#include "../common/save_title_changer.h"
#include "../undo_commands/change_entities_component_command.h"
#include "../widgets/main_window.h"
#include <QVBoxLayout>
#include <QSignalBlocker>
#include <vector>
#include <string>

LightComponentWidget::LightComponentWidget(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setLayout(new QVBoxLayout());
	layout()->setAlignment(Qt::AlignmentFlag::AlignTop);
	layout()->setContentsMargins(0, 0, 0, 0);
	layout()->addWidget(new ComponentTitleWidget(m_globalInfo, "Light"));
	std::vector<std::string> elementList = { "Directional", "Point", "Spot" };
	typeWidget = new ComboBoxWidget(m_globalInfo, "Type", elementList);
	layout()->addWidget(typeWidget);
	colorWidget = new ColorPickerWidget(m_globalInfo, "Color", nml::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	layout()->addWidget(colorWidget);
	directionWidget = new Vector3Widget(m_globalInfo, "Direction");
	layout()->addWidget(directionWidget);
	cutoffWidget = new Vector2Widget(m_globalInfo, "Cutoff");
	layout()->addWidget(cutoffWidget);
	layout()->addWidget(new SeparatorLine(m_globalInfo));

	connect(typeWidget, &ComboBoxWidget::elementSelected, this, &LightComponentWidget::onElementChanged);
	connect(colorWidget, &ColorPickerWidget::colorChanged, this, &LightComponentWidget::onColorChanged);
	connect(directionWidget, &Vector3Widget::valueChanged, this, &LightComponentWidget::onVec3Changed);
	connect(cutoffWidget, &Vector2Widget::valueChanged, this, &LightComponentWidget::onVec2Changed);
	connect(&globalInfo.signalEmitter, &SignalEmitter::selectEntitySignal, this, &LightComponentWidget::onEntitySelected);
	connect(&globalInfo.signalEmitter, &SignalEmitter::addEntityLightSignal, this, &LightComponentWidget::onAddEntityLight);
	connect(&globalInfo.signalEmitter, &SignalEmitter::removeEntityLightSignal, this, &LightComponentWidget::onRemoveEntityLight);
	connect(&globalInfo.signalEmitter, &SignalEmitter::changeEntityLightSignal, this, &LightComponentWidget::onChangeEntityLight);
}

void LightComponentWidget::updateWidgets(const Light& light) {
	{
		const QSignalBlocker signalBlocker(typeWidget->comboBox);
		typeWidget->comboBox->setCurrentText(QString::fromStdString(light.type));
	}
	colorWidget->setColor(nml::vec4(light.color, 1.0f));
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

void LightComponentWidget::onEntitySelected() {
	if ((m_globalInfo.currentEntityID != NO_ENTITY) && m_globalInfo.entities[m_globalInfo.currentEntityID].light.has_value()) {
		show();
		updateWidgets(m_globalInfo.entities[m_globalInfo.currentEntityID].light.value());
	}
	else {
		hide();
	}
}

void LightComponentWidget::onAddEntityLight(EntityID entityID) {
	if (entityID == m_globalInfo.currentEntityID) {
		const Light& light = m_globalInfo.entities[m_globalInfo.currentEntityID].light.value();
		updateWidgets(light);
		show();
	}

	SaveTitleChanger::change(m_globalInfo.mainWindow);
}

void LightComponentWidget::onRemoveEntityLight(EntityID entityID) {
	if (entityID == m_globalInfo.currentEntityID) {
		hide();
	}

	SaveTitleChanger::change(m_globalInfo.mainWindow);
}

void LightComponentWidget::onChangeEntityLight(EntityID entityID, const Light& light) {
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
		newLight.type = element;
	}
	updateComponent(m_globalInfo.currentEntityID, &newLight);
}

void LightComponentWidget::onColorChanged(const nml::vec4& color) {
	Light newLight = m_globalInfo.entities[m_globalInfo.currentEntityID].light.value();

	QObject* senderWidget = sender();
	if (senderWidget == colorWidget) {
		newLight.color = color;
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
