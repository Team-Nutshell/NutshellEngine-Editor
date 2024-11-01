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

void CollidableComponentWidget::updateComponents(const std::vector<EntityID>& entityIDs, std::vector<Collidable>& collidables) {
	std::vector<Component*> componentPointers;
	for (size_t i = 0; i < collidables.size(); i++) {
		componentPointers.push_back(&collidables[i]);
	}

	m_globalInfo.undoStack->push(new ChangeEntitiesComponentCommand(m_globalInfo, entityIDs, "Collidable", componentPointers));
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
	QObject* senderWidget = sender();

	std::vector<EntityID> entityIDs;
	std::vector<Collidable> newCollidables;

	std::set<EntityID> selectedEntityIDs = m_globalInfo.otherSelectedEntityIDs;
	selectedEntityIDs.insert(m_globalInfo.currentEntityID);
	for (EntityID selectedEntityID : selectedEntityIDs) {
		if (m_globalInfo.entities[selectedEntityID].collidable) {
			Collidable newCollidable = m_globalInfo.entities[selectedEntityID].collidable.value();

			if (senderWidget == typeWidget) {
				newCollidable.type = colliderTypeToType(element);
			}

			entityIDs.push_back(selectedEntityID);
			newCollidables.push_back(newCollidable);
		}
	}

	updateComponents(entityIDs, newCollidables);
}

void CollidableComponentWidget::onVec3Changed(const nml::vec3& value) {
	QObject* senderWidget = sender();

	std::vector<EntityID> entityIDs;
	std::vector<Collidable> newCollidables;

	Collidable newCollidable = m_globalInfo.entities[m_globalInfo.currentEntityID].collidable.value();

	uint8_t changedIndex = 255;
	if (senderWidget == centerWidget) {
		for (uint8_t i = 0; i < 3; i++) {
			if (newCollidable.center[i] != value[i]) {
				changedIndex = i;
				break;
			}
		}
		newCollidable.center = value;
	}
	else if (senderWidget == halfExtentWidget) {
		for (uint8_t i = 0; i < 3; i++) {
			if (newCollidable.halfExtent[i] != value[i]) {
				changedIndex = i;
				break;
			}
		}
		newCollidable.halfExtent = value;
	}
	else if (senderWidget == rotationWidget) {
		for (uint8_t i = 0; i < 3; i++) {
			if (newCollidable.rotation[i] != value[i]) {
				changedIndex = i;
				break;
			}
		}
		newCollidable.rotation = value;
	}
	else if (senderWidget == baseWidget) {
		for (uint8_t i = 0; i < 3; i++) {
			if (newCollidable.base[i] != value[i]) {
				changedIndex = i;
				break;
			}
		}
		newCollidable.base = value;
	}
	else if (senderWidget == tipWidget) {
		for (uint8_t i = 0; i < 3; i++) {
			if (newCollidable.tip[i] != value[i]) {
				changedIndex = i;
				break;
			}
		}
		newCollidable.tip = value;
	}
	entityIDs.push_back(m_globalInfo.currentEntityID);
	newCollidables.push_back(newCollidable);

	for (EntityID otherSelectedEntityID : m_globalInfo.otherSelectedEntityIDs) {
		if (m_globalInfo.entities[otherSelectedEntityID].collidable) {
			newCollidable = m_globalInfo.entities[otherSelectedEntityID].collidable.value();

			if (senderWidget == centerWidget) {
				newCollidable.center[changedIndex] = value[changedIndex];
			}
			else if (senderWidget == halfExtentWidget) {
				newCollidable.halfExtent[changedIndex] = value[changedIndex];
			}
			else if (senderWidget == rotationWidget) {
				newCollidable.rotation[changedIndex] = value[changedIndex];
			}
			else if (senderWidget == baseWidget) {
				newCollidable.base[changedIndex] = value[changedIndex];
			}
			else if (senderWidget == tipWidget) {
				newCollidable.tip[changedIndex] = value[changedIndex];
			}

			entityIDs.push_back(otherSelectedEntityID);
			newCollidables.push_back(newCollidable);
		}
	}

	updateComponents(entityIDs, newCollidables);
}

void CollidableComponentWidget::onScalarChanged(float value) {
	QObject* senderWidget = sender();

	std::vector<EntityID> entityIDs;
	std::vector<Collidable> newCollidables;

	std::set<EntityID> selectedEntityIDs = m_globalInfo.otherSelectedEntityIDs;
	selectedEntityIDs.insert(m_globalInfo.currentEntityID);
	for (EntityID selectedEntityID : selectedEntityIDs) {
		if (m_globalInfo.entities[selectedEntityID].collidable) {
			Collidable newCollidable = m_globalInfo.entities[selectedEntityID].collidable.value();

			if (senderWidget == radiusWidget) {
				newCollidable.radius = value;
			}

			entityIDs.push_back(selectedEntityID);
			newCollidables.push_back(newCollidable);
		}
	}

	updateComponents(entityIDs, newCollidables);
}

void CollidableComponentWidget::onFromRenderableButtonClicked() {
	std::vector<EntityID> entityIDs;
	std::vector<Collidable> newCollidables;

	std::set<EntityID> selectedEntityIDs = m_globalInfo.otherSelectedEntityIDs;
	selectedEntityIDs.insert(m_globalInfo.currentEntityID);
	for (EntityID selectedEntityID : selectedEntityIDs) {
		if (m_globalInfo.entities[selectedEntityID].renderable && ((!m_globalInfo.entities[selectedEntityID].renderable->modelPath.empty()) && (m_globalInfo.entities[selectedEntityID].renderable->primitiveIndex != NTSHENGN_NO_MODEL_PRIMITIVE))) {
			RendererResourceManager::Mesh& mesh = m_globalInfo.rendererResourceManager.models[m_globalInfo.entities[selectedEntityID].renderable->modelPath].primitives[m_globalInfo.entities[selectedEntityID].renderable->primitiveIndex].mesh;
			if (!mesh.collidersCalculated) {
				m_globalInfo.rendererResourceManager.loadMeshColliders(mesh);
			}

			Collidable newCollidable = m_globalInfo.entities[selectedEntityID].collidable.value();
			if (newCollidable.type == "Box") {
				newCollidable.center = mesh.obb.center;
				newCollidable.halfExtent = mesh.obb.halfExtent;
				newCollidable.rotation = mesh.obb.rotation;
			}
			else if (newCollidable.type == "Sphere") {
				newCollidable.center = mesh.sphere.center;
				newCollidable.radius = mesh.sphere.radius;
			}
			else if (newCollidable.type == "Capsule") {
				newCollidable.base = mesh.capsule.base;
				newCollidable.tip = mesh.capsule.tip;
				newCollidable.radius = mesh.capsule.radius;
			}
			entityIDs.push_back(selectedEntityID);
			newCollidables.push_back(newCollidable);

			ColliderMesh::update(m_globalInfo, selectedEntityID);

			if (m_globalInfo.currentEntityID) {
				updateWidgets(newCollidable);
			}
		}
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
