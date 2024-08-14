#pragma once
#include "../common/global_info.h"
#include "boolean_widget.h"
#include "scalar_widget.h"
#include <QWidget>

class RigidbodyComponentWidget : public QWidget {
	Q_OBJECT
public:
	RigidbodyComponentWidget(GlobalInfo& globalInfo);

private:
	void updateWidgets(const Rigidbody& rigidbody);
	void updateComponent(EntityID entityID, Component* component);

private slots:
	void onEntitySelected();
	void onEntityRigidbodyAdded(EntityID entityID);
	void onEntityRigidbodyRemoved(EntityID entityID);
	void onEntityRigidbodyChanged(EntityID entityID, const Rigidbody& rigidbody);
	void onBooleanChanged(bool boolean);
	void onScalarChanged(float value);

private:
	GlobalInfo& m_globalInfo;

public:
	BooleanWidget* isStaticWidget;
	BooleanWidget* isAffectedByConstantsWidget;
	BooleanWidget* lockRotationWidget;
	ScalarWidget* massWidget;
	ScalarWidget* inertiaWidget;
	ScalarWidget* restitutionWidget;
	ScalarWidget* staticFrictionWidget;
	ScalarWidget* dynamicFrictionWidget;
};