#include "collidable_component_widget.h"
#include "component_title_widget.h"
#include "separator_line.h"
#include "../undo_commands/change_entity_component_command.h"
#include <QVBoxLayout>
#include <QSignalBlocker>
#include <vector>
#include <string>

CollidableComponentWidget::CollidableComponentWidget(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setLayout(new QVBoxLayout());
	layout()->setAlignment(Qt::AlignmentFlag::AlignTop);
	layout()->setContentsMargins(0, 0, 0, 0);
	std::vector<std::string> elements = { "Box", "Sphere", "Capsule" };
	typeWidget = std::make_unique<ComboBoxWidget>(m_globalInfo, "Type", elements);
	layout()->addWidget(typeWidget.get());
	fromRenderableWidget = std::make_unique<BooleanWidget>(m_globalInfo, "From Renderable");
	layout()->addWidget(fromRenderableWidget.get());
	centerWidget = std::make_unique<Vector3Widget>(m_globalInfo, "Center");
	layout()->addWidget(centerWidget.get());
	radiusWidget = std::make_unique<ScalarWidget>(m_globalInfo, "Radius");
	layout()->addWidget(radiusWidget.get());
	halfExtentWidget = std::make_unique<Vector3Widget>(m_globalInfo, "Half Extent");
	layout()->addWidget(halfExtentWidget.get());
	rotationWidget = std::make_unique<Vector3Widget>(m_globalInfo, "Rotation");
	layout()->addWidget(rotationWidget.get());
	baseWidget = std::make_unique<Vector3Widget>(m_globalInfo, "Base");
	layout()->addWidget(baseWidget.get());
	tipWidget = std::make_unique<Vector3Widget>(m_globalInfo, "Tip");
	layout()->addWidget(tipWidget.get());

	connect(typeWidget.get(), &ComboBoxWidget::elementSelected, this, &CollidableComponentWidget::onElementUpdated);
	connect(centerWidget.get(), &Vector3Widget::valueChanged, this, &CollidableComponentWidget::onVec3Updated);
	connect(fromRenderableWidget.get(), &BooleanWidget::stateChanged, this, &CollidableComponentWidget::onBooleanUpdated);
	connect(radiusWidget.get(), &ScalarWidget::valueChanged, this, &CollidableComponentWidget::onScalarUpdated);
	connect(halfExtentWidget.get(), &Vector3Widget::valueChanged, this, &CollidableComponentWidget::onVec3Updated);
	connect(rotationWidget.get(), &Vector3Widget::valueChanged, this, &CollidableComponentWidget::onVec3Updated);
	connect(baseWidget.get(), &Vector3Widget::valueChanged, this, &CollidableComponentWidget::onVec3Updated);
	connect(tipWidget.get(), &Vector3Widget::valueChanged, this, &CollidableComponentWidget::onVec3Updated);
	connect(&globalInfo.signalEmitter, &SignalEmitter::selectEntitySignal, this, &CollidableComponentWidget::onSelectEntity);
	connect(&globalInfo.signalEmitter, &SignalEmitter::addEntityCollidableSignal, this, &CollidableComponentWidget::onAddEntityCollidable);
	connect(&globalInfo.signalEmitter, &SignalEmitter::removeEntityCollidableSignal, this, &CollidableComponentWidget::onRemoveEntityCollidable);
	connect(&globalInfo.signalEmitter, &SignalEmitter::changeEntityCollidableSignal, this, &CollidableComponentWidget::onChangeEntityCollidable);
}

void CollidableComponentWidget::updateWidgets(const Collidable& collidable) {
	{
		const QSignalBlocker signalBlocker(typeWidget->comboBox.get());
		typeWidget->comboBox->setCurrentText(QString::fromStdString(collidable.type));
	}
	{
		const QSignalBlocker signalBlocker(fromRenderableWidget->checkBox.get());
		fromRenderableWidget->checkBox->setChecked(collidable.fromRenderable);
	}
	centerWidget->value = collidable.center;
	centerWidget->xLineEdit->setText(QString::number(collidable.center.x, 'g', 3));
	centerWidget->yLineEdit->setText(QString::number(collidable.center.y, 'g', 3));
	centerWidget->zLineEdit->setText(QString::number(collidable.center.z, 'g', 3));
	if (((collidable.type == "Box") || (collidable.type == "Sphere")) && !collidable.fromRenderable) {
		centerWidget->setEnabled(true);
	}
	else {
		centerWidget->setEnabled(false);
	}
	radiusWidget->value = collidable.radius;
	radiusWidget->valueLineEdit->setText(QString::number(collidable.radius, 'g', 3));
	if (((collidable.type == "Sphere") || (collidable.type == "Radius")) && !collidable.fromRenderable) {
		radiusWidget->setEnabled(true);
	}
	else {
		radiusWidget->setEnabled(false);
	}
	halfExtentWidget->value = collidable.halfExtent;
	halfExtentWidget->xLineEdit->setText(QString::number(collidable.halfExtent.x, 'g', 3));
	halfExtentWidget->yLineEdit->setText(QString::number(collidable.halfExtent.y, 'g', 3));
	halfExtentWidget->zLineEdit->setText(QString::number(collidable.halfExtent.z, 'g', 3));
	if (collidable.type == "Box" && !collidable.fromRenderable) {
		halfExtentWidget->setEnabled(true);
	}
	else {
		halfExtentWidget->setEnabled(false);
	}
	rotationWidget->value = collidable.rotation;
	rotationWidget->xLineEdit->setText(QString::number(collidable.rotation.x, 'g', 3));
	rotationWidget->yLineEdit->setText(QString::number(collidable.rotation.y, 'g', 3));
	rotationWidget->zLineEdit->setText(QString::number(collidable.rotation.z, 'g', 3));
	if (collidable.type == "Box" && !collidable.fromRenderable) {
		rotationWidget->setEnabled(true);
	}
	else {
		rotationWidget->setEnabled(false);
	}
	baseWidget->value = collidable.base;
	baseWidget->xLineEdit->setText(QString::number(collidable.base.x, 'g', 3));
	baseWidget->yLineEdit->setText(QString::number(collidable.base.y, 'g', 3));
	baseWidget->zLineEdit->setText(QString::number(collidable.base.z, 'g', 3));
	if (collidable.type == "Capsule" && !collidable.fromRenderable) {
		baseWidget->setEnabled(true);
	}
	else {
		baseWidget->setEnabled(false);
	}
	tipWidget->value = collidable.tip;
	tipWidget->xLineEdit->setText(QString::number(collidable.tip.x, 'g', 3));
	tipWidget->yLineEdit->setText(QString::number(collidable.tip.y, 'g', 3));
	tipWidget->zLineEdit->setText(QString::number(collidable.tip.z, 'g', 3));
	if (collidable.type == "Capsule" && !collidable.fromRenderable) {
		tipWidget->setEnabled(true);
	}
	else {
		tipWidget->setEnabled(false);
	}
}

void CollidableComponentWidget::onSelectEntity() {
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
		Collidable collidable = m_globalInfo.entities[m_globalInfo.currentEntityID].collidable.value();
		updateWidgets(collidable);
		show();
	}
}

void CollidableComponentWidget::onRemoveEntityCollidable(EntityID entityID) {
	if (entityID == m_globalInfo.currentEntityID) {
		hide();
	}
}

void CollidableComponentWidget::onChangeEntityCollidable(EntityID entityID, const Collidable& collidable) {
	if (sender() != this) {
		if (entityID == m_globalInfo.currentEntityID) {
			updateWidgets(collidable);
		}
	}
}

void CollidableComponentWidget::onElementUpdated(const std::string& element) {
	Collidable newCollidable = m_globalInfo.entities[m_globalInfo.currentEntityID].collidable.value();
	if (sender() == typeWidget.get()) {
		newCollidable.type = element;
	}
	m_globalInfo.undoStack->push(new ChangeEntityComponentCommand(m_globalInfo, m_globalInfo.currentEntityID, "Collidable", &newCollidable));
}

void CollidableComponentWidget::onVec3Updated(const nml::vec3& value) {
	Collidable newCollidable = m_globalInfo.entities[m_globalInfo.currentEntityID].collidable.value();
	if (sender() == centerWidget.get()) {
		newCollidable.center = value;
	}
	else if (sender() == halfExtentWidget.get()) {
		newCollidable.halfExtent = value;
	}
	else if (sender() == rotationWidget.get()) {
		newCollidable.rotation = value;
	}
	else if (sender() == baseWidget.get()) {
		newCollidable.base = value;
	}
	else if (sender() == tipWidget.get()) {
		newCollidable.tip = value;
	}
	m_globalInfo.undoStack->push(new ChangeEntityComponentCommand(m_globalInfo, m_globalInfo.currentEntityID, "Collidable", &newCollidable));
}

void CollidableComponentWidget::onBooleanUpdated(bool boolean) {
	Collidable newCollidable = m_globalInfo.entities[m_globalInfo.currentEntityID].collidable.value();
	if (sender() == fromRenderableWidget.get()) {
		newCollidable.fromRenderable = boolean;
	}
	m_globalInfo.undoStack->push(new ChangeEntityComponentCommand(m_globalInfo, m_globalInfo.currentEntityID, "Collidable", &newCollidable));
}

void CollidableComponentWidget::onScalarUpdated(float value) {
	Collidable newCollidable = m_globalInfo.entities[m_globalInfo.currentEntityID].collidable.value();
	if (sender() == radiusWidget.get()) {
		newCollidable.radius = value;
	}
	m_globalInfo.undoStack->push(new ChangeEntityComponentCommand(m_globalInfo, m_globalInfo.currentEntityID, "Collidable", &newCollidable));
}
