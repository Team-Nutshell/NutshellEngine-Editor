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
	void toggleBackfaceCulling();
	void toggleCameraVisibility();
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
	void onBackfaceCullingToggled(bool backfaceCullingEnabled);
	void onCameraVisibilityToggled(bool showCameras);
	void onCameraProjectionSwitched(bool cameraProjectionOrthographic);

private:
	GlobalInfo& m_globalInfo;

	QAction* m_toggleCurrentEntityVisibilityAction;
	QAction* m_toggleBackfaceCullingAction;
	QAction* m_toggleCamerasVisibilityAction;
	QAction* m_switchCameraProjectionAction;
	QAction* m_resetCameraAction;
	QAction* m_orthographicCameraToXMAction;
	QAction* m_orthographicCameraToXPAction;
	QAction* m_orthographicCameraToYMAction;
	QAction* m_orthographicCameraToYPAction;
	QAction* m_orthographicCameraToZMAction;
	QAction* m_orthographicCameraToZPAction;

	bool m_backfaceCullingEnabled = false;
	bool m_showCameras = false;
	bool m_cameraProjectionOrthographic = false;
};