#include "light_component_widget.h"
#include "component_title_widget.h"
#include "separator_line.h"
#include "../common/save_title_changer.h"
#include "../undo_commands/change_entity_component_command.h"
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
	connect(&globalInfo.signalEmitter, &SignalEmitter::selectEntitySignal, this, &LightComponentWidget::onSelectEntity);
	connect(&globalInfo.signalEmitter, &SignalEmitter::addEntityLightSignal, this, &LightComponentWidget::onAddEntityLight);
	connect(&globalInfo.signalEmitter, &SignalEmitter::removeEntityLightSignal, this, &LightComponentWidget::onRemoveEntityLight);
	connect(&globalInfo.signalEmitter, &SignalEmitter::changeEntityLightSignal, this, &LightComponentWidget::onChangeEntityLight);
}

void LightComponentWidget::updateWidgets(const Light& light) {
	{
		const QSignalBlocker signalBlocker(typeWidget->comboBox);
		typeWidget->comboBox->setCurrentText(QString::fromStdString(light.type));
	}
	colorWidget->updateColor(nml::vec4(light.color, 1.0f));
	directionWidget->value = light.direction;
	directionWidget->xLineEdit->setText(QString::number(light.direction.x, 'f', 3));
	directionWidget->yLineEdit->setText(QString::number(light.direction.y, 'f', 3));
	directionWidget->zLineEdit->setText(QString::number(light.direction.z, 'f', 3));
	if ((light.type == "Directional") || (light.type == "Spot")) {
		directionWidget->setEnabled(true);
	}
	else {
		directionWidget->setEnabled(false);
	}
	cutoffWidget->value = light.cutoff;
	cutoffWidget->xLineEdit->setText(QString::number(light.cutoff.x, 'f', 3));
	cutoffWidget->yLineEdit->setText(QString::number(light.cutoff.y, 'f', 3));
	if (light.type == "Spot") {
		cutoffWidget->setEnabled(true);
	}
	else {
		cutoffWidget->setEnabled(false);
	}
}

void LightComponentWidget::onSelectEntity() {
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

	SaveTitleChanger::change(reinterpret_cast<MainWindow*>(m_globalInfo.mainWindow));
}

void LightComponentWidget::onRemoveEntityLight(EntityID entityID) {
	if (entityID == m_globalInfo.currentEntityID) {
		hide();
	}

	SaveTitleChanger::change(reinterpret_cast<MainWindow*>(m_globalInfo.mainWindow));
}

void LightComponentWidget::onChangeEntityLight(EntityID entityID, const Light& light) {
	QObject* senderWidget = sender();
	if (senderWidget != this) {
		if (entityID == m_globalInfo.currentEntityID) {
			updateWidgets(light);
		}
	}

	SaveTitleChanger::change(reinterpret_cast<MainWindow*>(m_globalInfo.mainWindow));
}

void LightComponentWidget::onElementChanged(const std::string& element) {
	Light newLight = m_globalInfo.entities[m_globalInfo.currentEntityID].light.value();

	QObject* senderWidget = sender();
	if (senderWidget == typeWidget) {
		newLight.type = element;
	}
	m_globalInfo.undoStack->push(new ChangeEntityComponentCommand(m_globalInfo, m_globalInfo.currentEntityID, "Light", &newLight));
}

void LightComponentWidget::onColorChanged(const nml::vec4& color) {
	Light newLight = m_globalInfo.entities[m_globalInfo.currentEntityID].light.value();

	QObject* senderWidget = sender();
	if (senderWidget == colorWidget) {
		newLight.color = color;
	}
	m_globalInfo.undoStack->push(new ChangeEntityComponentCommand(m_globalInfo, m_globalInfo.currentEntityID, "Light", &newLight));
}

void LightComponentWidget::onVec3Changed(const nml::vec3& value) {
	Light newLight = m_globalInfo.entities[m_globalInfo.currentEntityID].light.value();

	QObject* senderWidget = sender();
	if (senderWidget == directionWidget) {
		newLight.direction = value;
	}
	m_globalInfo.undoStack->push(new ChangeEntityComponentCommand(m_globalInfo, m_globalInfo.currentEntityID, "Light", &newLight));
}

void LightComponentWidget::onVec2Changed(const nml::vec2& value) {
	Light newLight = m_globalInfo.entities[m_globalInfo.currentEntityID].light.value();

	QObject* senderWidget = sender();
	if (senderWidget == cutoffWidget) {
		newLight.cutoff = value;
	}
	m_globalInfo.undoStack->push(new ChangeEntityComponentCommand(m_globalInfo, m_globalInfo.currentEntityID, "Light", &newLight));
}
