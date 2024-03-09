#include "rigidbody_component_widget.h"
#include "component_title_widget.h"
#include "separator_line.h"
#include "../undo_commands/change_entity_component_command.h"
#include <QVBoxLayout>
#include <QSignalBlocker>

RigidbodyComponentWidget::RigidbodyComponentWidget(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setLayout(new QVBoxLayout());
	layout()->setAlignment(Qt::AlignmentFlag::AlignTop);
	layout()->setContentsMargins(0, 0, 0, 0);
	isStaticWidget = std::make_unique<BooleanWidget>(m_globalInfo, "Is Static");
	layout()->addWidget(isStaticWidget.get());
	isAffectedByConstantsWidget = std::make_unique<BooleanWidget>(m_globalInfo, "Is Affected By Constants");
	layout()->addWidget(isAffectedByConstantsWidget.get());
	lockRotationWidget = std::make_unique<BooleanWidget>(m_globalInfo, "Lock Rotation");
	layout()->addWidget(lockRotationWidget.get());
	massWidget = std::make_unique<ScalarWidget>(m_globalInfo, "Mass");
	layout()->addWidget(massWidget.get());
	inertiaWidget = std::make_unique<ScalarWidget>(m_globalInfo, "Inertia");
	layout()->addWidget(inertiaWidget.get());
	restitutionWidget = std::make_unique<ScalarWidget>(m_globalInfo, "Restitution");
	layout()->addWidget(restitutionWidget.get());
	staticFrictionWidget = std::make_unique<ScalarWidget>(m_globalInfo, "Static Friction");
	layout()->addWidget(staticFrictionWidget.get());
	dynamicFrictionWidget = std::make_unique<ScalarWidget>(m_globalInfo, "Dynamic Friction");
	layout()->addWidget(dynamicFrictionWidget.get());

	connect(isStaticWidget.get(), &BooleanWidget::stateChanged, this, &RigidbodyComponentWidget::onBooleanUpdated);
	connect(isAffectedByConstantsWidget.get(), &BooleanWidget::stateChanged, this, &RigidbodyComponentWidget::onBooleanUpdated);
	connect(lockRotationWidget.get(), &BooleanWidget::stateChanged, this, &RigidbodyComponentWidget::onBooleanUpdated);
	connect(massWidget.get(), &ScalarWidget::valueChanged, this, &RigidbodyComponentWidget::onScalarUpdated);
	connect(inertiaWidget.get(), &ScalarWidget::valueChanged, this, &RigidbodyComponentWidget::onScalarUpdated);
	connect(restitutionWidget.get(), &ScalarWidget::valueChanged, this, &RigidbodyComponentWidget::onScalarUpdated);
	connect(staticFrictionWidget.get(), &ScalarWidget::valueChanged, this, &RigidbodyComponentWidget::onScalarUpdated);
	connect(dynamicFrictionWidget.get(), &ScalarWidget::valueChanged, this, &RigidbodyComponentWidget::onScalarUpdated);
	connect(&globalInfo.signalEmitter, &SignalEmitter::selectEntitySignal, this, &RigidbodyComponentWidget::onSelectEntity);
	connect(&globalInfo.signalEmitter, &SignalEmitter::addEntityRigidbodySignal, this, &RigidbodyComponentWidget::onAddEntityRigidbody);
	connect(&globalInfo.signalEmitter, &SignalEmitter::removeEntityRigidbodySignal, this, &RigidbodyComponentWidget::onRemoveEntityRigidbody);
	connect(&globalInfo.signalEmitter, &SignalEmitter::changeEntityRigidbodySignal, this, &RigidbodyComponentWidget::onChangeEntityRigidbody);
}

void RigidbodyComponentWidget::updateWidgets(const Rigidbody& rigidbody) {
	{
		const QSignalBlocker signalBlocker(isStaticWidget->checkBox.get());
		isStaticWidget->checkBox->setChecked(rigidbody.isStatic);
	}
	{
		const QSignalBlocker signalBlocker(isAffectedByConstantsWidget->checkBox.get());
		isAffectedByConstantsWidget->checkBox->setChecked(rigidbody.isAffectedByConstants);
	}
	{
		const QSignalBlocker signalBlocker(lockRotationWidget->checkBox.get());
		lockRotationWidget->checkBox->setChecked(rigidbody.lockRotation);
	}
	massWidget->value = rigidbody.mass;
	massWidget->valueLineEdit->setText(QString::number(rigidbody.mass, 'g', 3));
	inertiaWidget->value = rigidbody.mass;
	inertiaWidget->valueLineEdit->setText(QString::number(rigidbody.inertia, 'g', 3));
	restitutionWidget->value = rigidbody.restitution;
	restitutionWidget->valueLineEdit->setText(QString::number(rigidbody.restitution, 'g', 3));
	staticFrictionWidget->value = rigidbody.staticFriction;
	staticFrictionWidget->valueLineEdit->setText(QString::number(rigidbody.staticFriction, 'g', 3));
	dynamicFrictionWidget->value = rigidbody.dynamicFriction;
	dynamicFrictionWidget->valueLineEdit->setText(QString::number(rigidbody.dynamicFriction, 'g', 3));
}

void RigidbodyComponentWidget::onSelectEntity() {
	if ((m_globalInfo.currentEntityID != NO_ENTITY) && m_globalInfo.entities[m_globalInfo.currentEntityID].rigidbody.has_value()) {
		show();
		updateWidgets(m_globalInfo.entities[m_globalInfo.currentEntityID].rigidbody.value());
	}
	else {
		hide();
	}
}

void RigidbodyComponentWidget::onAddEntityRigidbody(EntityID entityID) {
	if (entityID == m_globalInfo.currentEntityID) {
		Rigidbody rigidbody = m_globalInfo.entities[m_globalInfo.currentEntityID].rigidbody.value();
		updateWidgets(rigidbody);
		show();
	}
}

void RigidbodyComponentWidget::onRemoveEntityRigidbody(EntityID entityID) {
	if (entityID == m_globalInfo.currentEntityID) {
		hide();
	}
}

void RigidbodyComponentWidget::onChangeEntityRigidbody(EntityID entityID, const Rigidbody& rigidbody) {
	if (sender() != this) {
		if (entityID == m_globalInfo.currentEntityID) {
			updateWidgets(rigidbody);
		}
	}
}

void RigidbodyComponentWidget::onBooleanUpdated(bool boolean) {
	Rigidbody newRigidbody = m_globalInfo.entities[m_globalInfo.currentEntityID].rigidbody.value();
	if (sender() == isStaticWidget.get()) {
		newRigidbody.isStatic = boolean;
	}
	else if (sender() == isAffectedByConstantsWidget.get()) {
		newRigidbody.isAffectedByConstants = boolean;
	}
	else if (sender() == lockRotationWidget.get()) {
		newRigidbody.lockRotation = boolean;
	}
	m_globalInfo.undoStack->push(new ChangeEntityComponentCommand(m_globalInfo, m_globalInfo.currentEntityID, "Rigidbody", &newRigidbody));
}

void RigidbodyComponentWidget::onScalarUpdated(float value) {
	Rigidbody newRigidbody = m_globalInfo.entities[m_globalInfo.currentEntityID].rigidbody.value();
	if (sender() == massWidget.get()) {
		newRigidbody.mass = value;
	}
	else if (sender() == inertiaWidget.get()) {
		newRigidbody.inertia = value;
	}
	else if (sender() == restitutionWidget.get()) {
		newRigidbody.restitution = value;
	}
	else if (sender() == staticFrictionWidget.get()) {
		newRigidbody.staticFriction = value;
	}
	else if (sender() == dynamicFrictionWidget.get()) {
		newRigidbody.dynamicFriction = value;
	}
	m_globalInfo.undoStack->push(new ChangeEntityComponentCommand(m_globalInfo, m_globalInfo.currentEntityID, "Rigidbody", &newRigidbody));
}
