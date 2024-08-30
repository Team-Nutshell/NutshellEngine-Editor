#pragma once
#include "../common/global_info.h"
#include <QMenu>
#include <QAction>

class ViewMenu : public QMenu {
	Q_OBJECT
public:
	ViewMenu(GlobalInfo& globalInfo);

private:
	void toggleCurrentEntityVisibility();
	void toggleGridVisibility();
	void toggleBackfaceCulling();
	void toggleCamerasVisibility();
	void toggleLighting();
	void toggleCollidersVisibility();
	void switchCameraProjection();
	void resetCamera();
	void orthographicCameraToXM();
	void orthographicCameraToXP();
	void orthographicCameraToYM();
	void orthographicCameraToYP();
	void orthographicCameraToZM();
	void orthographicCameraToZP();

	void setGridVisibility(bool showGrid);
	void setBackfaceCulling(bool backfaceCullingEnabled);
	void setCamerasVisibility(bool showCameras);
	void setLighting(bool lightingEnabled);
	void setCollidersVisibility(bool showCameras);

	void save();

private slots:
	void onEntitySelected();
	void onEntityVisibilityToggled(EntityID entityID, bool isEntityVisible);
	void onCameraProjectionSwitched(bool cameraProjectionOrthographic);

private:
	GlobalInfo& m_globalInfo;

	bool m_cameraProjectionOrthographic = false;

public:
	QAction* toggleCurrentEntityVisibilityAction;
	QAction* toggleGridVisibilityAction;
	QAction* toggleBackfaceCullingAction;
	QAction* toggleCamerasVisibilityAction;
	QAction* toggleLightingAction;
	QAction* toggleCollidersVisibilityAction;
	QAction* switchCameraProjectionAction;
	QAction* resetCameraAction;
	QAction* orthographicCameraToXMAction;
	QAction* orthographicCameraToXPAction;
	QAction* orthographicCameraToYMAction;
	QAction* orthographicCameraToYPAction;
	QAction* orthographicCameraToZMAction;
	QAction* orthographicCameraToZPAction;
};