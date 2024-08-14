#include "rigidbody_component_widget.h"
#include "component_title_widget.h"
#include "separator_line.h"
#include "../common/save_title_changer.h"
#include "../undo_commands/change_entities_component_command.h"
#include "../widgets/main_window.h"
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

	connect(isStaticWidget, &BooleanWidget::stateChanged, this, &RigidbodyComponentWidget::onBooleanChanged);
	connect(isAffectedByConstantsWidget, &BooleanWidget::stateChanged, this, &RigidbodyComponentWidget::onBooleanChanged);
	connect(lockRotationWidget, &BooleanWidget::stateChanged, this, &RigidbodyComponentWidget::onBooleanChanged);
	connect(massWidget, &ScalarWidget::valueChanged, this, &RigidbodyComponentWidget::onScalarChanged);
	connect(inertiaWidget, &ScalarWidget::valueChanged, this, &RigidbodyComponentWidget::onScalarChanged);
	connect(restitutionWidget, &ScalarWidget::valueChanged, this, &RigidbodyComponentWidget::onScalarChanged);
	connect(staticFrictionWidget, &ScalarWidget::valueChanged, this, &RigidbodyComponentWidget::onScalarChanged);
	connect(dynamicFrictionWidget, &ScalarWidget::valueChanged, this, &RigidbodyComponentWidget::onScalarChanged);
	connect(&globalInfo.signalEmitter, &SignalEmitter::selectEntitySignal, this, &RigidbodyComponentWidget::onEntitySelected);
	connect(&globalInfo.signalEmitter, &SignalEmitter::addEntityRigidbodySignal, this, &RigidbodyComponentWidget::onEntityRigidbodyAdded);
	connect(&globalInfo.signalEmitter, &SignalEmitter::removeEntityRigidbodySignal, this, &RigidbodyComponentWidget::onEntityRigidbodyRemoved);
	connect(&globalInfo.signalEmitter, &SignalEmitter::changeEntityRigidbodySignal, this, &RigidbodyComponentWidget::onEntityRigidbodyChanged);
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
	massWidget->setValue(rigidbody.mass);
	inertiaWidget->setValue(rigidbody.inertia);
	restitutionWidget->setValue(rigidbody.restitution);
	staticFrictionWidget->setValue(rigidbody.staticFriction);
	dynamicFrictionWidget->setValue(rigidbody.dynamicFriction);
}

void RigidbodyComponentWidget::updateComponent(EntityID entityID, Component* component) {
	m_globalInfo.undoStack->push(new ChangeEntitiesComponentCommand(m_globalInfo, { entityID }, "Rigidbody", { component }));
}

void RigidbodyComponentWidget::onEntitySelected() {
	if ((m_globalInfo.currentEntityID != NO_ENTITY) && m_globalInfo.entities[m_globalInfo.currentEntityID].rigidbody.has_value()) {
		show();
		updateWidgets(m_globalInfo.entities[m_globalInfo.currentEntityID].rigidbody.value());
	}
	else {
		hide();
	}
}

void RigidbodyComponentWidget::onEntityRigidbodyAdded(EntityID entityID) {
	if (entityID == m_globalInfo.currentEntityID) {
		const Rigidbody& rigidbody = m_globalInfo.entities[m_globalInfo.currentEntityID].rigidbody.value();
		updateWidgets(rigidbody);
		show();
	}

	SaveTitleChanger::change(m_globalInfo.mainWindow);
}

void RigidbodyComponentWidget::onEntityRigidbodyRemoved(EntityID entityID) {
	if (entityID == m_globalInfo.currentEntityID) {
		hide();
	}

	SaveTitleChanger::change(m_globalInfo.mainWindow);
}

void RigidbodyComponentWidget::onEntityRigidbodyChanged(EntityID entityID, const Rigidbody& rigidbody) {
	QObject* senderWidget = sender();
	if (senderWidget != this) {
		if (entityID == m_globalInfo.currentEntityID) {
			updateWidgets(rigidbody);
		}
	}

	SaveTitleChanger::change(m_globalInfo.mainWindow);
}

void RigidbodyComponentWidget::onBooleanChanged(bool boolean) {
	Rigidbody newRigidbody = m_globalInfo.entities[m_globalInfo.currentEntityID].rigidbody.value();

	QObject* senderWidget = sender();
	if (senderWidget == isStaticWidget) {
		newRigidbody.isStatic = boolean;
	}
	else if (senderWidget == isAffectedByConstantsWidget) {
		newRigidbody.isAffectedByConstants = boolean;
	}
	else if (senderWidget == lockRotationWidget) {
		newRigidbody.lockRotation = boolean;
	}
	updateComponent(m_globalInfo.currentEntityID, &newRigidbody);
}

void RigidbodyComponentWidget::onScalarChanged(float value) {
	Rigidbody newRigidbody = m_globalInfo.entities[m_globalInfo.currentEntityID].rigidbody.value();

	QObject* senderWidget = sender();
	if (senderWidget == massWidget) {
		newRigidbody.mass = value;
	}
	else if (senderWidget == inertiaWidget) {
		newRigidbody.inertia = value;
	}
	else if (senderWidget == restitutionWidget) {
		newRigidbody.restitution = value;
	}
	else if (senderWidget == staticFrictionWidget) {
		newRigidbody.staticFriction = value;
	}
	else if (senderWidget == dynamicFrictionWidget) {
		newRigidbody.dynamicFriction = value;
	}
	updateComponent(m_globalInfo.currentEntityID, &newRigidbody);
}
