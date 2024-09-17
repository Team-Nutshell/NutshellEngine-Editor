#include "collidable_component_widget.h"
#include "component_title_widget.h"
#include "separator_line.h"
#include "main_window.h"
#include "../common/save_title_changer.h"
#include "../undo_commands/change_entities_component_command.h"
#include "../renderer/collider_mesh.h"
#include <QVBoxLayout>
#include <QSignalBlocker>
#include <vector>
#include <string>

CollidableComponentWidget::CollidableComponentWidget(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setLayout(new QVBoxLayout());
	layout()->setAlignment(Qt::AlignmentFlag::AlignTop);
	layout()->setContentsMargins(0, 0, 0, 0);
	layout()->addWidget(new ComponentTitleWidget(m_globalInfo, m_globalInfo.localization.getString("component_collidable")));
	std::vector<std::string> elements = { m_globalInfo.localization.getString("component_collidable_type_box"), m_globalInfo.localization.getString("component_collidable_type_sphere"), m_globalInfo.localization.getString("component_collidable_type_capsule") };
	typeWidget = new ComboBoxWidget(m_globalInfo, m_globalInfo.localization.getString("component_collidable_type"), elements);
	layout()->addWidget(typeWidget);
	centerWidget = new Vector3Widget(m_globalInfo, m_globalInfo.localization.getString("component_collidable_center"));
	layout()->addWidget(centerWidget);
	radiusWidget = new ScalarWidget(m_globalInfo, m_globalInfo.localization.getString("component_collidable_radius"));
	radiusWidget->setMin(0.0f);
	layout()->addWidget(radiusWidget);
	halfExtentWidget = new Vector3Widget(m_globalInfo, m_globalInfo.localization.getString("component_collidable_half_extent"));
	layout()->addWidget(halfExtentWidget);
	rotationWidget = new Vector3Widget(m_globalInfo, m_globalInfo.localization.getString("component_collidable_rotation"));
	layout()->addWidget(rotationWidget);
	baseWidget = new Vector3Widget(m_globalInfo, m_globalInfo.localization.getString("component_collidable_base"));
	layout()->addWidget(baseWidget);
	tipWidget = new Vector3Widget(m_globalInfo, m_globalInfo.localization.getString("component_collidable_tip"));
	layout()->addWidget(tipWidget);
	fromRenderableWidget = new QPushButton(QString::fromStdString(m_globalInfo.localization.getString("component_collidable_from_renderable")));
	layout()->addWidget(fromRenderableWidget);
	layout()->addWidget(new SeparatorLine(m_globalInfo));

	connect(typeWidget, &ComboBoxWidget::elementSelected, this, &CollidableComponentWidget::onElementChanged);
	connect(centerWidget, &Vector3Widget::valueChanged, this, &CollidableComponentWidget::onVec3Changed);
	connect(radiusWidget, &ScalarWidget::valueChanged, this, &CollidableComponentWidget::onScalarChanged);
	connect(halfExtentWidget, &Vector3Widget::valueChanged, this, &CollidableComponentWidget::onVec3Changed);
	connect(rotationWidget, &Vector3Widget::valueChanged, this, &CollidableComponentWidget::onVec3Changed);
	connect(baseWidget, &Vector3Widget::valueChanged, this, &CollidableComponentWidget::onVec3Changed);
	connect(tipWidget, &Vector3Widget::valueChanged, this, &CollidableComponentWidget::onVec3Changed);
	connect(fromRenderableWidget, &QPushButton::clicked, this, &CollidableComponentWidget::onFromRenderableButtonClicked);
	connect(&globalInfo.signalEmitter, &SignalEmitter::createEntitySignal, this, &CollidableComponentWidget::onEntityCreated);
	connect(&globalInfo.signalEmitter, &SignalEmitter::selectEntitySignal, this, &CollidableComponentWidget::onEntitySelected);
	connect(&globalInfo.signalEmitter, &SignalEmitter::addEntityCollidableSignal, this, &CollidableComponentWidget::onEntityCollidableAdded);
	connect(&globalInfo.signalEmitter, &SignalEmitter::removeEntityCollidableSignal, this, &CollidableComponentWidget::onEntityCollidableRemoved);
	connect(&globalInfo.signalEmitter, &SignalEmitter::changeEntityCollidableSignal, this, &CollidableComponentWidget::onEntityCollidableChanged);
	connect(&globalInfo.signalEmitter, &SignalEmitter::addEntityRenderableSignal, this, &CollidableComponentWidget::onEntityRenderableAdded);
	connect(&globalInfo.signalEmitter, &SignalEmitter::removeEntityRenderableSignal, this, &CollidableComponentWidget::onEntityRenderableRemoved);
	connect(&globalInfo.signalEmitter, &SignalEmitter::changeEntityRenderableSignal, this, &CollidableComponentWidget::onEntityRenderableChanged);
}

void CollidableComponentWidget::updateWidgets(const Collidable& collidable) {
	typeWidget->setElementByText(typeToColliderType(collidable.type));
	centerWidget->setValue(collidable.center);
	if ((collidable.type == "Box") || (collidable.type == "Sphere")) {
		centerWidget->setEnabled(true);
	}
	else {
		centerWidget->setEnabled(false);
	}
	radiusWidget->setValue(collidable.radius);
	if ((collidable.type == "Sphere") || (collidable.type == "Capsule")) {
		radiusWidget->setEnabled(true);
	}
	else {
		radiusWidget->setEnabled(false);
	}
	halfExtentWidget->setValue(collidable.halfExtent);
	if (collidable.type == "Box") {
		halfExtentWidget->setEnabled(true);
	}
	else {
		halfExtentWidget->setEnabled(false);
	}
	rotationWidget->setValue(collidable.rotation);
	if (collidable.type == "Box") {
		rotationWidget->setEnabled(true);
	}
	else {
		rotationWidget->setEnabled(false);
	}
	baseWidget->setValue(collidable.base);
	if (collidable.type == "Capsule") {
		baseWidget->setEnabled(true);
	}
	else {
		baseWidget->setEnabled(false);
	}
	tipWidget->setValue(collidable.tip);
	if (collidable.type == "Capsule") {
		tipWidget->setEnabled(true);
	}
	else {
		tipWidget->setEnabled(false);
	}
	updateFromRenderableWidget();
}

void CollidableComponentWidget::updateComponent(EntityID entityID, Component* component) {
	m_globalInfo.undoStack->push(new ChangeEntitiesComponentCommand(m_globalInfo, { entityID }, "Collidable", { component }));
}

void CollidableComponentWidget::updateFromRenderableWidget() {
	if (m_globalInfo.entities[m_globalInfo.currentEntityID].renderable && ((!m_globalInfo.entities[m_globalInfo.currentEntityID].renderable->modelPath.empty()) && (m_globalInfo.entities[m_globalInfo.currentEntityID].renderable->primitiveIndex != NTSHENGN_NO_MODEL_PRIMITIVE))) {
		fromRenderableWidget->setEnabled(true);
	}
	else {
		fromRenderableWidget->setEnabled(false);
	}
}

std::string CollidableComponentWidget::colliderTypeToType(const std::string& colliderType) {
	if (colliderType == m_globalInfo.localization.getString("component_collidable_type_box")) {
		return "Box";
	}
	else if (colliderType == m_globalInfo.localization.getString("component_collidable_type_sphere")) {
		return "Sphere";
	}
	else if (colliderType == m_globalInfo.localization.getString("component_collidable_type_capsule")) {
		return "Capsule";
	}
	else {
		return "Unknown";
	}
}

std::string CollidableComponentWidget::typeToColliderType(const std::string& type) {
	if (type == "Box") {
		return m_globalInfo.localization.getString("component_collidable_type_box");
	}
	else if (type == "Sphere") {
		return m_globalInfo.localization.getString("component_collidable_type_sphere");
	}
	else if (type == "Capsule") {
		return m_globalInfo.localization.getString("component_collidable_type_capsule");
	}
	else {
		return "Unknown";
	}
}

void CollidableComponentWidget::onEntityCreated(EntityID entityID) {
	Entity& entity = m_globalInfo.entities[entityID];
	if (entity.collidable) {
		ColliderMesh::update(m_globalInfo, entityID);
	}
}

void CollidableComponentWidget::onEntitySelected() {
	if ((m_globalInfo.currentEntityID != NO_ENTITY) && m_globalInfo.entities[m_globalInfo.currentEntityID].collidable.has_value()) {
		show();
		updateWidgets(m_globalInfo.entities[m_globalInfo.currentEntityID].collidable.value());
	}
	else {
		hide();
	}
}

void CollidableComponentWidget::onEntityCollidableAdded(EntityID entityID) {
	if (entityID == m_globalInfo.currentEntityID) {
		const Collidable& collidable = m_globalInfo.entities[m_globalInfo.currentEntityID].collidable.value();
		updateWidgets(collidable);
		show();
	}

	ColliderMesh::update(m_globalInfo, entityID);

	SaveTitleChanger::change(m_globalInfo.mainWindow);
}

void CollidableComponentWidget::onEntityCollidableRemoved(EntityID entityID) {
	if (entityID == m_globalInfo.currentEntityID) {
		hide();
	}

	SaveTitleChanger::change(m_globalInfo.mainWindow);
}

void CollidableComponentWidget::onEntityCollidableChanged(EntityID entityID, const Collidable& collidable) {
	QObject* senderWidget = sender();
	if (senderWidget != this) {
		if (entityID == m_globalInfo.currentEntityID) {
			updateWidgets(collidable);
		}
	}

	ColliderMesh::update(m_globalInfo, entityID);

	SaveTitleChanger::change(m_globalInfo.mainWindow);
}

void CollidableComponentWidget::onElementChanged(const std::string& element) {
	Collidable newCollidable = m_globalInfo.entities[m_globalInfo.currentEntityID].collidable.value();

	QObject* senderWidget = sender();
	if (senderWidget == typeWidget) {
		newCollidable.type = colliderTypeToType(element);
	}
	updateComponent(m_globalInfo.currentEntityID, &newCollidable);
}

void CollidableComponentWidget::onVec3Changed(const nml::vec3& value) {
	Collidable newCollidable = m_globalInfo.entities[m_globalInfo.currentEntityID].collidable.value();

	QObject* senderWidget = sender();
	if (senderWidget == centerWidget) {
		newCollidable.center = value;
	}
	else if (senderWidget == halfExtentWidget) {
		newCollidable.halfExtent = value;
	}
	else if (senderWidget == rotationWidget) {
		newCollidable.rotation = value;
	}
	else if (senderWidget == baseWidget) {
		newCollidable.base = value;
	}
	else if (senderWidget == tipWidget) {
		newCollidable.tip = value;
	}
	updateComponent(m_globalInfo.currentEntityID, &newCollidable);
}

void CollidableComponentWidget::onScalarChanged(float value) {
	Collidable newCollidable = m_globalInfo.entities[m_globalInfo.currentEntityID].collidable.value();

	QObject* senderWidget = sender();
	if (senderWidget == radiusWidget) {
		newCollidable.radius = value;
	}
	updateComponent(m_globalInfo.currentEntityID, &newCollidable);
}

void CollidableComponentWidget::onFromRenderableButtonClicked() {
	if (m_globalInfo.entities[m_globalInfo.currentEntityID].renderable && ((!m_globalInfo.entities[m_globalInfo.currentEntityID].renderable->modelPath.empty()) && (m_globalInfo.entities[m_globalInfo.currentEntityID].renderable->primitiveIndex != NTSHENGN_NO_MODEL_PRIMITIVE))) {
		RendererResourceManager::Mesh& mesh = m_globalInfo.rendererResourceManager.models[m_globalInfo.entities[m_globalInfo.currentEntityID].renderable->modelPath].primitives[m_globalInfo.entities[m_globalInfo.currentEntityID].renderable->primitiveIndex].mesh;
		if (!mesh.collidersCalculated) {
			m_globalInfo.rendererResourceManager.loadMeshColliders(mesh);
		}

		const Collidable& collidable = m_globalInfo.entities[m_globalInfo.currentEntityID].collidable.value();
		Collidable newCollidable = collidable;
		if (collidable.type == "Box") {
			newCollidable.center = mesh.obb.center;
			newCollidable.halfExtent = mesh.obb.halfExtent;
			newCollidable.rotation = mesh.obb.rotation;
		}
		else if (collidable.type == "Sphere") {
			newCollidable.center = mesh.sphere.center;
			newCollidable.radius = mesh.sphere.radius;
		}
		else if (collidable.type == "Capsule") {
			newCollidable.base = mesh.capsule.base;
			newCollidable.tip = mesh.capsule.tip;
			newCollidable.radius = mesh.capsule.radius;
		}
		updateComponent(m_globalInfo.currentEntityID, &newCollidable);
		updateWidgets(newCollidable);

		ColliderMesh::update(m_globalInfo, m_globalInfo.currentEntityID);
	}
}

void CollidableComponentWidget::onEntityRenderableAdded(EntityID entityID) {
	if (entityID != m_globalInfo.currentEntityID) {
		return;
	}

	updateFromRenderableWidget();
}

void CollidableComponentWidget::onEntityRenderableRemoved(EntityID entityID) {
	if (entityID != m_globalInfo.currentEntityID) {
		return;
	}

	fromRenderableWidget->setEnabled(false);
}

void CollidableComponentWidget::onEntityRenderableChanged(EntityID entityID) {
	if (entityID != m_globalInfo.currentEntityID) {
		return;
	}

	updateFromRenderableWidget();
}
