#include "transform_component_widget.h"
#include "component_title_widget.h"
#include "separator_line.h"
#include "main_window.h"
#include "../common/save_title_changer.h"
#include "../undo_commands/change_entities_component_command.h"
#include <QVBoxLayout>

TransformComponentWidget::TransformComponentWidget(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setLayout(new QVBoxLayout());
	layout()->setAlignment(Qt::AlignmentFlag::AlignTop);
	layout()->setContentsMargins(0, 0, 0, 0);
	layout()->addWidget(new SeparatorLine(m_globalInfo));
	layout()->addWidget(new ComponentTitleWidget(m_globalInfo, "Transform"));
	positionWidget = new Vector3Widget(m_globalInfo, "Position");
	layout()->addWidget(positionWidget);
	rotationWidget = new Vector3Widget(m_globalInfo, "Rotation");
	layout()->addWidget(rotationWidget);
	scaleWidget = new Vector3Widget(m_globalInfo, "Scale");
	layout()->addWidget(scaleWidget);
	layout()->addWidget(new SeparatorLine(m_globalInfo));

	connect(positionWidget, &Vector3Widget::valueChanged, this, &TransformComponentWidget::onVec3Changed);
	connect(rotationWidget, &Vector3Widget::valueChanged, this, &TransformComponentWidget::onVec3Changed);
	connect(scaleWidget, &Vector3Widget::valueChanged, this, &TransformComponentWidget::onVec3Changed);
	connect(&globalInfo.signalEmitter, &SignalEmitter::selectEntitySignal, this, &TransformComponentWidget::onEntitySelected);
	connect(&globalInfo.signalEmitter, &SignalEmitter::changeEntityTransformSignal, this, &TransformComponentWidget::onEntityTransformChanged);
}

void TransformComponentWidget::updateWidgets(const Transform& transform) {
	positionWidget->setValue(transform.position);
	rotationWidget->setValue(transform.rotation);
	scaleWidget->setValue(transform.scale);
}

void TransformComponentWidget::updateComponent(EntityID entityID, Component* component) {
	m_globalInfo.undoStack->push(new ChangeEntitiesComponentCommand(m_globalInfo, { entityID }, "Transform", { component }));
}

void TransformComponentWidget::onEntitySelected() {
	if (m_globalInfo.currentEntityID != NO_ENTITY) {
		show();
		updateWidgets(m_globalInfo.entities[m_globalInfo.currentEntityID].transform);
	}
	else {
		hide();
	}
}

void TransformComponentWidget::onEntityTransformChanged(EntityID entityID, const Transform& transform) {
	QObject* senderWidget = sender();
	if (senderWidget != this) {
		if (entityID == m_globalInfo.currentEntityID) {
			updateWidgets(transform);
		}
	}

	SaveTitleChanger::change(m_globalInfo.mainWindow);
}

void TransformComponentWidget::onVec3Changed(const nml::vec3& value) {
	Transform newTransform = m_globalInfo.entities[m_globalInfo.currentEntityID].transform;

	QObject* senderWidget = sender();
	if (senderWidget == positionWidget) {
		newTransform.position = value;
	}
	else if (senderWidget == rotationWidget) {
		newTransform.rotation = value;
	}
	else if (senderWidget == scaleWidget) {
		newTransform.scale = value;
	}
	updateComponent(m_globalInfo.currentEntityID, &newTransform);
}