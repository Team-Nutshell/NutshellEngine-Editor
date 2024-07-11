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
	layout()->addWidget(new ComponentTitleWidget(m_globalInfo, "Rigidbody"));
	isStaticWidget = new BooleanWidget(m_globalInfo, "Is Static");
	layout()->addWidget(isStaticWidget);
	isAffectedByConstantsWidget = new BooleanWidget(m_globalInfo, "Is Affected By Constants");
	layout()->addWidget(isAffectedByConstantsWidget);
	lockRotationWidget = new BooleanWidget(m_globalInfo, "Lock Rotation");
	layout()->addWidget(lockRotationWidget);
	massWidget = new ScalarWidget(m_globalInfo, "Mass");
	layout()->addWidget(massWidget);
	inertiaWidget = new ScalarWidget(m_globalInfo, "Inertia");
	layout()->addWidget(inertiaWidget);
	restitutionWidget = new ScalarWidget(m_globalInfo, "Restitution");
	layout()->addWidget(restitutionWidget);
	staticFrictionWidget = new ScalarWidget(m_globalInfo, "Static Friction");
	layout()->addWidget(staticFrictionWidget);
	dynamicFrictionWidget = new ScalarWidget(m_globalInfo, "Dynamic Friction");
	layout()->addWidget(dynamicFrictionWidget);
	layout()->addWidget(new SeparatorLine(m_globalInfo));

	connect(isStaticWidget, &BooleanWidget::stateChanged, this, &RigidbodyComponentWidget::onBooleanUpdated);
	connect(isAffectedByConstantsWidget, &BooleanWidget::stateChanged, this, &RigidbodyComponentWidget::onBooleanUpdated);
	connect(lockRotationWidget, &BooleanWidget::stateChanged, this, &RigidbodyComponentWidget::onBooleanUpdated);
	connect(massWidget, &ScalarWidget::valueChanged, this, &RigidbodyComponentWidget::onScalarUpdated);
	connect(inertiaWidget, &ScalarWidget::valueChanged, this, &RigidbodyComponentWidget::onScalarUpdated);
	connect(restitutionWidget, &ScalarWidget::valueChanged, this, &RigidbodyComponentWidget::onScalarUpdated);
	connect(staticFrictionWidget, &ScalarWidget::valueChanged, this, &RigidbodyComponentWidget::onScalarUpdated);
	connect(dynamicFrictionWidget, &ScalarWidget::valueChanged, this, &RigidbodyComponentWidget::onScalarUpdated);
	connect(&globalInfo.signalEmitter, &SignalEmitter::selectEntitySignal, this, &RigidbodyComponentWidget::onSelectEntity);
	connect(&globalInfo.signalEmitter, &SignalEmitter::addEntityRigidbodySignal, this, &RigidbodyComponentWidget::onAddEntityRigidbody);
	connect(&globalInfo.signalEmitter, &SignalEmitter::removeEntityRigidbodySignal, this, &RigidbodyComponentWidget::onRemoveEntityRigidbody);
	connect(&globalInfo.signalEmitter, &SignalEmitter::changeEntityRigidbodySignal, this, &RigidbodyComponentWidget::onChangeEntityRigidbody);
}

void RigidbodyComponentWidget::updateWidgets(const Rigidbody& rigidbody) {
	{
		const QSignalBlocker signalBlocker(isStaticWidget->checkBox);
		isStaticWidget->checkBox->setChecked(rigidbody.isStatic);
	}
	{
		const QSignalBlocker signalBlocker(isAffectedByConstantsWidget->checkBox);
		isAffectedByConstantsWidget->checkBox->setChecked(rigidbody.isAffectedByConstants);
	}
	{
		const QSignalBlocker signalBlocker(lockRotationWidget->checkBox);
		lockRotationWidget->checkBox->setChecked(rigidbody.lockRotation);
	}
	massWidget->value = rigidbody.mass;
	massWidget->valueLineEdit->setText(QString::number(rigidbody.mass, 'f', 3));
	inertiaWidget->value = rigidbody.mass;
	inertiaWidget->valueLineEdit->setText(QString::number(rigidbody.inertia, 'f', 3));
	restitutionWidget->value = rigidbody.restitution;
	restitutionWidget->valueLineEdit->setText(QString::number(rigidbody.restitution, 'f', 3));
	staticFrictionWidget->value = rigidbody.staticFriction;
	staticFrictionWidget->valueLineEdit->setText(QString::number(rigidbody.staticFriction, 'f', 3));
	dynamicFrictionWidget->value = rigidbody.dynamicFriction;
	dynamicFrictionWidget->valueLineEdit->setText(QString::number(rigidbody.dynamicFriction, 'f', 3));
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
		const Rigidbody& rigidbody = m_globalInfo.entities[m_globalInfo.currentEntityID].rigidbody.value();
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
	if (sender() == isStaticWidget) {
		newRigidbody.isStatic = boolean;
	}
	else if (sender() == isAffectedByConstantsWidget) {
		newRigidbody.isAffectedByConstants = boolean;
	}
	else if (sender() == lockRotationWidget) {
		newRigidbody.lockRotation = boolean;
	}
	m_globalInfo.undoStack->push(new ChangeEntityComponentCommand(m_globalInfo, m_globalInfo.currentEntityID, "Rigidbody", &newRigidbody));
}

void RigidbodyComponentWidget::onScalarUpdated(float value) {
	Rigidbody newRigidbody = m_globalInfo.entities[m_globalInfo.currentEntityID].rigidbody.value();
	if (sender() == massWidget) {
		newRigidbody.mass = value;
	}
	else if (sender() == inertiaWidget) {
		newRigidbody.inertia = value;
	}
	else if (sender() == restitutionWidget) {
		newRigidbody.restitution = value;
	}
	else if (sender() == staticFrictionWidget) {
		newRigidbody.staticFriction = value;
	}
	else if (sender() == dynamicFrictionWidget) {
		newRigidbody.dynamicFriction = value;
	}
	m_globalInfo.undoStack->push(new ChangeEntityComponentCommand(m_globalInfo, m_globalInfo.currentEntityID, "Rigidbody", &newRigidbody));
}
