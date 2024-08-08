#include "collidable_component_widget.h"
#include "component_title_widget.h"
#include "separator_line.h"
#include "../common/save_title_changer.h"
#include "../undo_commands/change_entity_component_command.h"
#include "../renderer/collider_mesh.h"
#include "../widgets/main_window.h"
#include <QVBoxLayout>
#include <QSignalBlocker>
#include <vector>
#include <string>

CollidableComponentWidget::CollidableComponentWidget(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setLayout(new QVBoxLayout());
	layout()->setAlignment(Qt::AlignmentFlag::AlignTop);
	layout()->setContentsMargins(0, 0, 0, 0);
	layout()->addWidget(new ComponentTitleWidget(m_globalInfo, "Collidable"));
	std::vector<std::string> elements = { "Box", "Sphere", "Capsule" };
	typeWidget = new ComboBoxWidget(m_globalInfo, "Type", elements);
	layout()->addWidget(typeWidget);
	centerWidget = new Vector3Widget(m_globalInfo, "Center");
	layout()->addWidget(centerWidget);
	radiusWidget = new ScalarWidget(m_globalInfo, "Radius");
	layout()->addWidget(radiusWidget);
	halfExtentWidget = new Vector3Widget(m_globalInfo, "Half Extent");
	layout()->addWidget(halfExtentWidget);
	rotationWidget = new Vector3Widget(m_globalInfo, "Rotation");
	layout()->addWidget(rotationWidget);
	baseWidget = new Vector3Widget(m_globalInfo, "Base");
	layout()->addWidget(baseWidget);
	tipWidget = new Vector3Widget(m_globalInfo, "Tip");
	layout()->addWidget(tipWidget);
	fromRenderableWidget = new QPushButton("From Renderable");
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
	connect(&globalInfo.signalEmitter, &SignalEmitter::selectEntitySignal, this, &CollidableComponentWidget::onEntitySelected);
	connect(&globalInfo.signalEmitter, &SignalEmitter::addEntityCollidableSignal, this, &CollidableComponentWidget::onAddEntityCollidable);
	connect(&globalInfo.signalEmitter, &SignalEmitter::removeEntityCollidableSignal, this, &CollidableComponentWidget::onRemoveEntityCollidable);
	connect(&globalInfo.signalEmitter, &SignalEmitter::changeEntityCollidableSignal, this, &CollidableComponentWidget::onChangeEntityCollidable);
	connect(&globalInfo.signalEmitter, &SignalEmitter::addEntityRenderableSignal, this, &CollidableComponentWidget::onAddEntityRenderable);
	connect(&globalInfo.signalEmitter, &SignalEmitter::removeEntityRenderableSignal, this, &CollidableComponentWidget::onRemoveEntityRenderable);
	connect(&globalInfo.signalEmitter, &SignalEmitter::changeEntityRenderableSignal, this, &CollidableComponentWidget::onChangeEntityRenderable);
}

void CollidableComponentWidget::updateWidgets(const Collidable& collidable) {
	{
		const QSignalBlocker signalBlocker(typeWidget->comboBox);
		typeWidget->comboBox->setCurrentText(QString::fromStdString(collidable.type));
	}
	centerWidget->value = collidable.center;
	centerWidget->xLineEdit->setText(QString::number(collidable.center.x, 'f', 3));
	centerWidget->yLineEdit->setText(QString::number(collidable.center.y, 'f', 3));
	centerWidget->zLineEdit->setText(QString::number(collidable.center.z, 'f', 3));
	if ((collidable.type == "Box") || (collidable.type == "Sphere")) {
		centerWidget->setEnabled(true);
	}
	else {
		centerWidget->setEnabled(false);
	}
	radiusWidget->value = collidable.radius;
	radiusWidget->valueLineEdit->setText(QString::number(collidable.radius, 'f', 3));
	if ((collidable.type == "Sphere") || (collidable.type == "Capsule")) {
		radiusWidget->setEnabled(true);
	}
	else {
		radiusWidget->setEnabled(false);
	}
	halfExtentWidget->value = collidable.halfExtent;
	halfExtentWidget->xLineEdit->setText(QString::number(collidable.halfExtent.x, 'f', 3));
	halfExtentWidget->yLineEdit->setText(QString::number(collidable.halfExtent.y, 'f', 3));
	halfExtentWidget->zLineEdit->setText(QString::number(collidable.halfExtent.z, 'f', 3));
	if (collidable.type == "Box") {
		halfExtentWidget->setEnabled(true);
	}
	else {
		halfExtentWidget->setEnabled(false);
	}
	rotationWidget->value = collidable.rotation;
	rotationWidget->xLineEdit->setText(QString::number(collidable.rotation.x, 'f', 3));
	rotationWidget->yLineEdit->setText(QString::number(collidable.rotation.y, 'f', 3));
	rotationWidget->zLineEdit->setText(QString::number(collidable.rotation.z, 'f', 3));
	if (collidable.type == "Box") {
		rotationWidget->setEnabled(true);
	}
	else {
		rotationWidget->setEnabled(false);
	}
	baseWidget->value = collidable.base;
	baseWidget->xLineEdit->setText(QString::number(collidable.base.x, 'f', 3));
	baseWidget->yLineEdit->setText(QString::number(collidable.base.y, 'f', 3));
	baseWidget->zLineEdit->setText(QString::number(collidable.base.z, 'f', 3));
	if (collidable.type == "Capsule") {
		baseWidget->setEnabled(true);
	}
	else {
		baseWidget->setEnabled(false);
	}
	tipWidget->value = collidable.tip;
	tipWidget->xLineEdit->setText(QString::number(collidable.tip.x, 'f', 3));
	tipWidget->yLineEdit->setText(QString::number(collidable.tip.y, 'f', 3));
	tipWidget->zLineEdit->setText(QString::number(collidable.tip.z, 'f', 3));
	if (collidable.type == "Capsule") {
		tipWidget->setEnabled(true);
	}
	else {
		tipWidget->setEnabled(false);
	}
	updateFromRenderableWidget();
}

void CollidableComponentWidget::updateFromRenderableWidget() {
	if (m_globalInfo.entities[m_globalInfo.currentEntityID].renderable && ((m_globalInfo.entities[m_globalInfo.currentEntityID].renderable->modelPath != "") && (m_globalInfo.entities[m_globalInfo.currentEntityID].renderable->primitiveIndex != NTSHENGN_NO_MODEL_PRIMITIVE))) {
		fromRenderableWidget->setEnabled(true);
	}
	else {
		fromRenderableWidget->setEnabled(false);
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

void CollidableComponentWidget::onAddEntityCollidable(EntityID entityID) {
	if (entityID == m_globalInfo.currentEntityID) {
		const Collidable& collidable = m_globalInfo.entities[m_globalInfo.currentEntityID].collidable.value();
		updateWidgets(collidable);
		show();
	}

	ColliderMesh::update(m_globalInfo, entityID);

	SaveTitleChanger::change(m_globalInfo.mainWindow);
}

void CollidableComponentWidget::onRemoveEntityCollidable(EntityID entityID) {
	if (entityID == m_globalInfo.currentEntityID) {
		hide();
	}

	SaveTitleChanger::change(m_globalInfo.mainWindow);
}

void CollidableComponentWidget::onChangeEntityCollidable(EntityID entityID, const Collidable& collidable) {
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
		newCollidable.type = element;
	}
	m_globalInfo.undoStack->push(new ChangeEntityComponentCommand(m_globalInfo, m_globalInfo.currentEntityID, "Collidable", &newCollidable));
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
	m_globalInfo.undoStack->push(new ChangeEntityComponentCommand(m_globalInfo, m_globalInfo.currentEntityID, "Collidable", &newCollidable));
}

void CollidableComponentWidget::onScalarChanged(float value) {
	Collidable newCollidable = m_globalInfo.entities[m_globalInfo.currentEntityID].collidable.value();

	QObject* senderWidget = sender();
	if (senderWidget == radiusWidget) {
		newCollidable.radius = value;
	}
	m_globalInfo.undoStack->push(new ChangeEntityComponentCommand(m_globalInfo, m_globalInfo.currentEntityID, "Collidable", &newCollidable));
}

void CollidableComponentWidget::onFromRenderableButtonClicked() {
	if (m_globalInfo.entities[m_globalInfo.currentEntityID].renderable && ((m_globalInfo.entities[m_globalInfo.currentEntityID].renderable->modelPath != "") && (m_globalInfo.entities[m_globalInfo.currentEntityID].renderable->primitiveIndex != NTSHENGN_NO_MODEL_PRIMITIVE))) {
		RendererResourceManager::Mesh& mesh = m_globalInfo.rendererResourceManager.models[m_globalInfo.entities[m_globalInfo.currentEntityID].renderable->modelPath].primitives[m_globalInfo.entities[m_globalInfo.currentEntityID].renderable->primitiveIndex].mesh;
		if (!mesh.collidersCalculated) {
			m_globalInfo.rendererResourceManager.loadMeshColliders(mesh);
		}

		Collidable& collidable = m_globalInfo.entities[m_globalInfo.currentEntityID].collidable.value();
		if (collidable.type == "Box") {
			collidable.center = mesh.obb.center;
			collidable.halfExtent = mesh.obb.halfExtent;
			collidable.rotation = mesh.obb.rotation;
		}
		else if (collidable.type == "Sphere") {
			collidable.center = mesh.sphere.center;
			collidable.radius = mesh.sphere.radius;
		}
		else if (collidable.type == "Capsule") {
			collidable.base = mesh.capsule.base;
			collidable.tip = mesh.capsule.tip;
			collidable.radius = mesh.capsule.radius;
		}
		updateWidgets(collidable);

		ColliderMesh::update(m_globalInfo, m_globalInfo.currentEntityID);
	}
}

void CollidableComponentWidget::onAddEntityRenderable(EntityID entityID) {
	if (entityID != m_globalInfo.currentEntityID) {
		return;
	}

	updateFromRenderableWidget();
}

void CollidableComponentWidget::onRemoveEntityRenderable(EntityID entityID) {
	if (entityID != m_globalInfo.currentEntityID) {
		return;
	}

	fromRenderableWidget->setEnabled(false);
}

void CollidableComponentWidget::onChangeEntityRenderable(EntityID entityID) {
	if (entityID != m_globalInfo.currentEntityID) {
		return;
	}

	updateFromRenderableWidget();
}
