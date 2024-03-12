#pragma once
#include "../common/common.h"
#include "boolean_widget.h"
#include "scalar_widget.h"
#include <QWidget>

class RigidbodyComponentWidget : public QWidget {
	Q_OBJECT
public:
	RigidbodyComponentWidget(GlobalInfo& globalInfo);

private:
	void updateWidgets(const Rigidbody& rigidbody);

private slots:
	void onSelectEntity();
	void onAddEntityRigidbody(EntityID entityID);
	void onRemoveEntityRigidbody(EntityID entityID);
	void onChangeEntityRigidbody(EntityID entityID, const Rigidbody& rigidbody);
	void onBooleanUpdated(bool boolean);
	void onScalarUpdated(float value);

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