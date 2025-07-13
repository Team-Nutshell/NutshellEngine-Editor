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
	layout()->setContentsMargins(2, 0, 2, 0);
	layout()->addWidget(new SeparatorLine());
	layout()->addWidget(new ComponentTitleWidget(m_globalInfo, m_globalInfo.localization.getString("component_transform")));
	positionWidget = new Vector3Widget(m_globalInfo, m_globalInfo.localization.getString("component_transform_position"));
	layout()->addWidget(positionWidget);
	rotationWidget = new Vector3Widget(m_globalInfo, m_globalInfo.localization.getString("component_transform_rotation"));
	layout()->addWidget(rotationWidget);
	scaleWidget = new Vector3Widget(m_globalInfo, m_globalInfo.localization.getString("component_transform_scale"));
	layout()->addWidget(scaleWidget);
	layout()->addWidget(new SeparatorLine());

	connect(positionWidget, &Vector3Widget::xChanged, this, &TransformComponentWidget::onXChanged);
	connect(positionWidget, &Vector3Widget::yChanged, this, &TransformComponentWidget::onYChanged);
	connect(positionWidget, &Vector3Widget::zChanged, this, &TransformComponentWidget::onZChanged);
	connect(rotationWidget, &Vector3Widget::xChanged, this, &TransformComponentWidget::onXChanged);
	connect(rotationWidget, &Vector3Widget::yChanged, this, &TransformComponentWidget::onYChanged);
	connect(rotationWidget, &Vector3Widget::zChanged, this, &TransformComponentWidget::onZChanged);
	connect(scaleWidget, &Vector3Widget::xChanged, this, &TransformComponentWidget::onXChanged);
	connect(scaleWidget, &Vector3Widget::yChanged, this, &TransformComponentWidget::onYChanged);
	connect(scaleWidget, &Vector3Widget::zChanged, this, &TransformComponentWidget::onZChanged);
	connect(&globalInfo.signalEmitter, &SignalEmitter::selectEntitySignal, this, &TransformComponentWidget::onEntitySelected);
	connect(&globalInfo.signalEmitter, &SignalEmitter::changeEntityTransformSignal, this, &TransformComponentWidget::onEntityTransformChanged);
}

void TransformComponentWidget::updateWidgets(const Transform& transform) {
	positionWidget->setValue(transform.position);
	rotationWidget->setValue(transform.rotation);
	scaleWidget->setValue(transform.scale);
}

void TransformComponentWidget::updateComponents(const std::vector<EntityID>& entityIDs, std::vector<Transform>& transforms) {
	std::vector<Component*> componentPointers;
	for (size_t i = 0; i < transforms.size(); i++) {
		componentPointers.push_back(&transforms[i]);
	}

	m_globalInfo.actionUndoStack->push(new ChangeEntitiesComponentCommand(m_globalInfo, entityIDs, "Transform", componentPointers));
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

void TransformComponentWidget::onXChanged(float value) {
	QObject* senderWidget = sender();

	std::vector<EntityID> entityIDs;
	std::vector<Transform> newTransforms;

	Transform newTransform = m_globalInfo.entities[m_globalInfo.currentEntityID].transform;
	if (senderWidget == positionWidget) {
		newTransform.position.x = value;
	}
	else if (senderWidget == rotationWidget) {
		newTransform.rotation.x = value;
	}
	else if (senderWidget == scaleWidget) {
		newTransform.scale.x = value;
	}
	entityIDs.push_back(m_globalInfo.currentEntityID);
	newTransforms.push_back(newTransform);

	for (EntityID otherSelectedEntityID : m_globalInfo.otherSelectedEntityIDs) {
		newTransform = m_globalInfo.entities[otherSelectedEntityID].transform;
		if (senderWidget == positionWidget) {
			newTransform.position.x = value;
		}
		else if (senderWidget == rotationWidget) {
			newTransform.rotation.x = value;
		}
		else if (senderWidget == scaleWidget) {
			newTransform.scale.x = value;
		}
		entityIDs.push_back(otherSelectedEntityID);
		newTransforms.push_back(newTransform);
	}

	updateComponents(entityIDs, newTransforms);
}

void TransformComponentWidget::onYChanged(float value) {
	QObject* senderWidget = sender();

	std::vector<EntityID> entityIDs;
	std::vector<Transform> newTransforms;

	Transform newTransform = m_globalInfo.entities[m_globalInfo.currentEntityID].transform;
	if (senderWidget == positionWidget) {
		newTransform.position.y = value;
	}
	else if (senderWidget == rotationWidget) {
		newTransform.rotation.y = value;
	}
	else if (senderWidget == scaleWidget) {
		newTransform.scale.y = value;
	}
	entityIDs.push_back(m_globalInfo.currentEntityID);
	newTransforms.push_back(newTransform);

	for (EntityID otherSelectedEntityID : m_globalInfo.otherSelectedEntityIDs) {
		newTransform = m_globalInfo.entities[otherSelectedEntityID].transform;
		if (senderWidget == positionWidget) {
			newTransform.position.y = value;
		}
		else if (senderWidget == rotationWidget) {
			newTransform.rotation.y = value;
		}
		else if (senderWidget == scaleWidget) {
			newTransform.scale.y = value;
		}
		entityIDs.push_back(otherSelectedEntityID);
		newTransforms.push_back(newTransform);
	}

	updateComponents(entityIDs, newTransforms);
}

void TransformComponentWidget::onZChanged(float value) {
	QObject* senderWidget = sender();

	std::vector<EntityID> entityIDs;
	std::vector<Transform> newTransforms;

	Transform newTransform = m_globalInfo.entities[m_globalInfo.currentEntityID].transform;
	if (senderWidget == positionWidget) {
		newTransform.position.z = value;
	}
	else if (senderWidget == rotationWidget) {
		newTransform.rotation.z = value;
	}
	else if (senderWidget == scaleWidget) {
		newTransform.scale.z = value;
	}
	entityIDs.push_back(m_globalInfo.currentEntityID);
	newTransforms.push_back(newTransform);

	for (EntityID otherSelectedEntityID : m_globalInfo.otherSelectedEntityIDs) {
		newTransform = m_globalInfo.entities[otherSelectedEntityID].transform;
		if (senderWidget == positionWidget) {
			newTransform.position.z = value;
		}
		else if (senderWidget == rotationWidget) {
			newTransform.rotation.z = value;
		}
		else if (senderWidget == scaleWidget) {
			newTransform.scale.z = value;
		}
		entityIDs.push_back(otherSelectedEntityID);
		newTransforms.push_back(newTransform);
	}

	updateComponents(entityIDs, newTransforms);
}
