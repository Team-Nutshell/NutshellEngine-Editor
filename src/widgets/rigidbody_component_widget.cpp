#include "rigidbody_component_widget.h"
#include "component_title_widget.h"
#include "separator_line.h"
#include "main_window.h"
#include "../common/save_title_changer.h"
#include "../undo_commands/change_entities_component_command.h"
#include <QVBoxLayout>
#include <QSignalBlocker>

RigidbodyComponentWidget::RigidbodyComponentWidget(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setLayout(new QVBoxLayout());
	layout()->setAlignment(Qt::AlignmentFlag::AlignTop);
	layout()->setContentsMargins(0, 0, 0, 0);
	layout()->addWidget(new ComponentTitleWidget(m_globalInfo, m_globalInfo.localization.getString("component_rigidbody")));
	isStaticWidget = new BooleanWidget(m_globalInfo, m_globalInfo.localization.getString("component_rigidbody_is_static"));
	layout()->addWidget(isStaticWidget);
	isAffectedByConstantsWidget = new BooleanWidget(m_globalInfo, m_globalInfo.localization.getString("component_rigidbody_is_affected_by_constants"));
	layout()->addWidget(isAffectedByConstantsWidget);
	lockRotationWidget = new BooleanWidget(m_globalInfo, m_globalInfo.localization.getString("component_rigidbody_lock_rotation"));
	layout()->addWidget(lockRotationWidget);
	massWidget = new ScalarWidget(m_globalInfo, m_globalInfo.localization.getString("component_rigidbody_mass"));
	layout()->addWidget(massWidget);
	inertiaWidget = new ScalarWidget(m_globalInfo, m_globalInfo.localization.getString("component_rigidbody_inertia"));
	layout()->addWidget(inertiaWidget);
	restitutionWidget = new ScalarWidget(m_globalInfo, m_globalInfo.localization.getString("component_rigidbody_restitution"));
	layout()->addWidget(restitutionWidget);
	staticFrictionWidget = new ScalarWidget(m_globalInfo, m_globalInfo.localization.getString("component_rigidbody_static_friction"));
	layout()->addWidget(staticFrictionWidget);
	dynamicFrictionWidget = new ScalarWidget(m_globalInfo, m_globalInfo.localization.getString("component_rigidbody_dynamic_friction"));
	layout()->addWidget(dynamicFrictionWidget);
	layout()->addWidget(new SeparatorLine());

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
	isStaticWidget->setValue(rigidbody.isStatic);
	isAffectedByConstantsWidget->setValue(rigidbody.isAffectedByConstants);
	lockRotationWidget->setValue(rigidbody.lockRotation);
	massWidget->setValue(rigidbody.mass);
	inertiaWidget->setValue(rigidbody.inertia);
	restitutionWidget->setValue(rigidbody.restitution);
	staticFrictionWidget->setValue(rigidbody.staticFriction);
	dynamicFrictionWidget->setValue(rigidbody.dynamicFriction);
}

void RigidbodyComponentWidget::updateComponents(const std::vector<EntityID>& entityIDs, std::vector<Rigidbody>& rigidbodies) {
	std::vector<Component*> componentPointers;
	for (size_t i = 0; i < rigidbodies.size(); i++) {
		componentPointers.push_back(&rigidbodies[i]);
	}

	m_globalInfo.actionUndoStack->push(new ChangeEntitiesComponentCommand(m_globalInfo, entityIDs, "Rigidbody", componentPointers));
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
	QObject* senderWidget = sender();

	std::vector<EntityID> entityIDs;
	std::vector<Rigidbody> newRigidbodies;

	std::set<EntityID> selectedEntityIDs = m_globalInfo.otherSelectedEntityIDs;
	selectedEntityIDs.insert(m_globalInfo.currentEntityID);
	for (EntityID selectedEntityID : selectedEntityIDs) {
		if (m_globalInfo.entities[selectedEntityID].rigidbody) {
			Rigidbody newRigidbody = m_globalInfo.entities[selectedEntityID].rigidbody.value();

			if (senderWidget == isStaticWidget) {
				newRigidbody.isStatic = boolean;
			}
			else if (senderWidget == isAffectedByConstantsWidget) {
				newRigidbody.isAffectedByConstants = boolean;
			}
			else if (senderWidget == lockRotationWidget) {
				newRigidbody.lockRotation = boolean;
			}

			entityIDs.push_back(selectedEntityID);
			newRigidbodies.push_back(newRigidbody);
		}
	}

	updateComponents(entityIDs, newRigidbodies);
}

void RigidbodyComponentWidget::onScalarChanged(float value) {
	QObject* senderWidget = sender();

	std::vector<EntityID> entityIDs;
	std::vector<Rigidbody> newRigidbodies;

	std::set<EntityID> selectedEntityIDs = m_globalInfo.otherSelectedEntityIDs;
	selectedEntityIDs.insert(m_globalInfo.currentEntityID);
	for (EntityID selectedEntityID : selectedEntityIDs) {
		if (m_globalInfo.entities[selectedEntityID].rigidbody) {
			Rigidbody newRigidbody = m_globalInfo.entities[selectedEntityID].rigidbody.value();

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

			entityIDs.push_back(selectedEntityID);
			newRigidbodies.push_back(newRigidbody);
		}
	}

	updateComponents(entityIDs, newRigidbodies);
}
