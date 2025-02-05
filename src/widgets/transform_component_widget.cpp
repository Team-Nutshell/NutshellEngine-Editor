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
	layout()->addWidget(new ComponentTitleWidget(m_globalInfo, m_globalInfo.localization.getString("component_transform")));
	positionWidget = new Vector3Widget(m_globalInfo, m_globalInfo.localization.getString("component_transform_position"));
	layout()->addWidget(positionWidget);
	rotationWidget = new Vector3Widget(m_globalInfo, m_globalInfo.localization.getString("component_transform_rotation"));
	layout()->addWidget(rotationWidget);
	scaleWidget = new Vector3Widget(m_globalInfo, m_globalInfo.localization.getString("component_transform_scale"));
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

void TransformComponentWidget::onVec3Changed(const nml::vec3& value) {
	QObject* senderWidget = sender();

	std::vector<EntityID> entityIDs;
	std::vector<Transform> newTransforms;

	Transform newTransform = m_globalInfo.entities[m_globalInfo.currentEntityID].transform;

	uint8_t changedIndex = 255;
	if (senderWidget == positionWidget) {
		for (uint8_t i = 0; i < 3; i++) {
			if (newTransform.position[i] != value[i]) {
				changedIndex = i;
				break;
			}
		}
		newTransform.position = value;
	}
	else if (senderWidget == rotationWidget) {
		for (uint8_t i = 0; i < 3; i++) {
			if (newTransform.rotation[i] != value[i]) {
				changedIndex = i;
				break;
			}
		}
		newTransform.rotation = value;
	}
	else if (senderWidget == scaleWidget) {
		for (uint8_t i = 0; i < 3; i++) {
			if (newTransform.scale[i] != value[i]) {
				changedIndex = i;
				break;
			}
		}
		newTransform.scale = value;
	}
	entityIDs.push_back(m_globalInfo.currentEntityID);
	newTransforms.push_back(newTransform);

	for (EntityID otherSelectedEntityID : m_globalInfo.otherSelectedEntityIDs) {
		newTransform = m_globalInfo.entities[otherSelectedEntityID].transform;

		if (senderWidget == positionWidget) {
			newTransform.position[changedIndex] = value[changedIndex];
		}
		else if (senderWidget == rotationWidget) {
			newTransform.rotation[changedIndex] = value[changedIndex];
		}
		else if (senderWidget == scaleWidget) {
			newTransform.scale[changedIndex] = value[changedIndex];
		}

		entityIDs.push_back(otherSelectedEntityID);
		newTransforms.push_back(newTransform);
	}

	updateComponents(entityIDs, newTransforms);
}