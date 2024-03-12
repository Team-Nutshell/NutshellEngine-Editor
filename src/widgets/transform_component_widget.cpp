#include "transform_component_widget.h"
#include "component_title_widget.h"
#include "separator_line.h"
#include "../undo_commands/change_entity_component_command.h"
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

	connect(positionWidget, &Vector3Widget::valueChanged, this, &TransformComponentWidget::onVec3Updated);
	connect(rotationWidget, &Vector3Widget::valueChanged, this, &TransformComponentWidget::onVec3Updated);
	connect(scaleWidget, &Vector3Widget::valueChanged, this, &TransformComponentWidget::onVec3Updated);
	connect(&globalInfo.signalEmitter, &SignalEmitter::selectEntitySignal, this, &TransformComponentWidget::onSelectEntity);
	connect(&globalInfo.signalEmitter, &SignalEmitter::changeEntityTransformSignal, this, &TransformComponentWidget::onChangeEntityTransform);
}

void TransformComponentWidget::updateWidgets(const Transform& transform) {
	positionWidget->value = transform.position;
	positionWidget->xLineEdit->setText(QString::number(transform.position.x, 'g', 3));
	positionWidget->yLineEdit->setText(QString::number(transform.position.y, 'g', 3));
	positionWidget->zLineEdit->setText(QString::number(transform.position.z, 'g', 3));
	rotationWidget->value = transform.rotation;
	rotationWidget->xLineEdit->setText(QString::number(transform.rotation.x, 'g', 3));
	rotationWidget->yLineEdit->setText(QString::number(transform.rotation.y, 'g', 3));
	rotationWidget->zLineEdit->setText(QString::number(transform.rotation.z, 'g', 3));
	scaleWidget->value = transform.scale;
	scaleWidget->xLineEdit->setText(QString::number(transform.scale.x, 'g', 3));
	scaleWidget->yLineEdit->setText(QString::number(transform.scale.y, 'g', 3));
	scaleWidget->zLineEdit->setText(QString::number(transform.scale.z, 'g', 3));
}

void TransformComponentWidget::onSelectEntity() {
	if (m_globalInfo.currentEntityID != NO_ENTITY) {
		show();
		updateWidgets(m_globalInfo.entities[m_globalInfo.currentEntityID].transform);
	}
	else {
		hide();
	}
}

void TransformComponentWidget::onChangeEntityTransform(EntityID entityID, const Transform& transform) {
	if (sender() != this) {
		if (entityID == m_globalInfo.currentEntityID) {
			updateWidgets(transform);
		}
	}
}

void TransformComponentWidget::onVec3Updated(const nml::vec3& value) {
	Transform newTransform = m_globalInfo.entities[m_globalInfo.currentEntityID].transform;
	if (sender() == positionWidget) {
		newTransform.position = value;
	}
	else if (sender() == rotationWidget) {
		newTransform.rotation = value;
	}
	else if (sender() == scaleWidget) {
		newTransform.scale = value;
	}
	m_globalInfo.undoStack->push(new ChangeEntityComponentCommand(m_globalInfo, m_globalInfo.currentEntityID, "Transform", &newTransform));
}