#pragma once
#include "../common/common.h"
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
	void onSelectEntity();
	void onCurrentEntityVisibilityToggled(bool isEntityVisible);
	void onGridVisibilityToggled(bool showGrid);
	void onBackfaceCullingToggled(bool backfaceCullingEnabled);
	void onCamerasVisibilityToggled(bool showCameras);
	void onLightingToggled(bool lightingEnabled);
	void onCollidersVisibilityToggled(bool showCameras);
	void onCameraProjectionSwitched(bool cameraProjectionOrthographic);

private:
	GlobalInfo& m_globalInfo;

	QAction* m_toggleCurrentEntityVisibilityAction;
	QAction* m_toggleGridVisibilityAction;
	QAction* m_toggleBackfaceCullingAction;
	QAction* m_toggleCamerasVisibilityAction;
	QAction* m_toggleLightingAction;
	QAction* m_toggleCollidersVisibilityAction;
	QAction* m_switchCameraProjectionAction;
	QAction* m_resetCameraAction;
	QAction* m_orthographicCameraToXMAction;
	QAction* m_orthographicCameraToXPAction;
	QAction* m_orthographicCameraToYMAction;
	QAction* m_orthographicCameraToYPAction;
	QAction* m_orthographicCameraToZMAction;
	QAction* m_orthographicCameraToZPAction;

	bool m_showGrid = true;
	bool m_backfaceCullingEnabled = false;
	bool m_showCameras = false;
	bool m_lightingEnabled = false;
	bool m_showColliders = false;
	bool m_cameraProjectionOrthographic = false;
};