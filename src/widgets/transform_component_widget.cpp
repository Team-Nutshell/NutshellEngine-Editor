#include "transform_component_widget.h"
#include "component_title_widget.h"
#include "separator_line.h"
#include "../common/save_title_changer.h"
#include "../undo_commands/change_entities_component_command.h"
#include "../widgets/main_window.h"
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
	connect(&globalInfo.signalEmitter, &SignalEmitter::changeEntityTransformSignal, this, &TransformComponentWidget::onChangeEntityTransform);
}

void TransformComponentWidget::updateWidgets(const Transform& transform) {
	positionWidget->value = transform.position;
	positionWidget->xLineEdit->setText(QString::number(transform.position.x, 'f', 3));
	positionWidget->yLineEdit->setText(QString::number(transform.position.y, 'f', 3));
	positionWidget->zLineEdit->setText(QString::number(transform.position.z, 'f', 3));
	rotationWidget->value = transform.rotation;
	rotationWidget->xLineEdit->setText(QString::number(transform.rotation.x, 'f', 3));
	rotationWidget->yLineEdit->setText(QString::number(transform.rotation.y, 'f', 3));
	rotationWidget->zLineEdit->setText(QString::number(transform.rotation.z, 'f', 3));
	scaleWidget->value = transform.scale;
	scaleWidget->xLineEdit->setText(QString::number(transform.scale.x, 'f', 3));
	scaleWidget->yLineEdit->setText(QString::number(transform.scale.y, 'f', 3));
	scaleWidget->zLineEdit->setText(QString::number(transform.scale.z, 'f', 3));
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

void TransformComponentWidget::onChangeEntityTransform(EntityID entityID, const Transform& transform) {
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