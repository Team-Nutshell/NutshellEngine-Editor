#pragma once
#include "../common/global_info.h"
#include "vector3_widget.h"
#include "scalar_widget.h"
#include <QWidget>

class CameraComponentWidget : public QWidget {
	Q_OBJECT
public:
	CameraComponentWidget(GlobalInfo& globalInfo);

private:
	void updateWidgets(const Camera& camera);
	void updateComponents(const std::vector<EntityID>& entityIDs, std::vector<Camera>& cameras);

private slots:
	void onEntitySelected();
	void onEntityCameraAdded(EntityID entityID);
	void onEntityCameraRemoved(EntityID entityID);
	void onEntityCameraChanged(EntityID entityID, const Camera& camera);
	void onVec3Changed(const nml::vec3& value);
	void onScalarChanged(float value);

private:
	GlobalInfo& m_globalInfo;

public:
	Vector3Widget* forwardWidget;
	Vector3Widget* upWidget;
	ScalarWidget* fovWidget;
	ScalarWidget* nearPlaneWidget;
	ScalarWidget* farPlaneWidget;
};