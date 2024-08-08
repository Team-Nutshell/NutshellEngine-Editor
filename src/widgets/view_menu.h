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

private slots:
	void onEntitySelected();
	void onEntityVisibilityToggled(EntityID entityID, bool isEntityVisible);
	void onGridVisibilityToggled(bool showGrid);
	void onBackfaceCullingToggled(bool backfaceCullingEnabled);
	void onCamerasVisibilityToggled(bool showCameras);
	void onLightingToggled(bool lightingEnabled);
	void onCollidersVisibilityToggled(bool showCameras);
	void onCameraProjectionSwitched(bool cameraProjectionOrthographic);

private:
	GlobalInfo& m_globalInfo;

	bool m_showGrid = true;
	bool m_backfaceCullingEnabled = false;
	bool m_showCameras = false;
	bool m_lightingEnabled = false;
	bool m_showColliders = false;
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