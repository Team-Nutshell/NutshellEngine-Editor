#pragma once
#include "../common/common.h"
#include "boolean_widget.h"
#include "scalar_widget.h"
#include <QWidget>
#include <memory>

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
	std::unique_ptr<BooleanWidget> isStaticWidget;
	std::unique_ptr<BooleanWidget> isAffectedByConstantsWidget;
	std::unique_ptr<BooleanWidget> lockRotationWidget;
	std::unique_ptr<ScalarWidget> massWidget;
	std::unique_ptr<ScalarWidget> inertiaWidget;
	std::unique_ptr<ScalarWidget> restitutionWidget;
	std::unique_ptr<ScalarWidget> staticFrictionWidget;
	std::unique_ptr<ScalarWidget> dynamicFrictionWidget;
};