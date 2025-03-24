#pragma once
#include "../../external/nlohmann/json.hpp"
#include "../../external/nml/include/nml.h"
#include <Qt>
#include <string>

struct RendererParameters {
	Qt::Key cameraForwardKey = Qt::Key_W;
	Qt::Key cameraBackwardKey = Qt::Key_S;
	Qt::Key cameraLeftKey = Qt::Key_A;
	Qt::Key cameraRightKey = Qt::Key_D;
	Qt::Key cameraUpKey = Qt::Key_Space;
	Qt::Key cameraDownKey = Qt::Key_Q;
	Qt::Key switchCameraProjectionKey = Qt::Key_P;
	Qt::Key resetCameraKey = Qt::Key_0;
	Qt::Key orthographicCameraToXMKey = Qt::Key_4;
	Qt::Key orthographicCameraToXPKey = Qt::Key_6;
	Qt::Key orthographicCameraToYMKey = Qt::Key_1;
	Qt::Key orthographicCameraToYPKey = Qt::Key_7;
	Qt::Key orthographicCameraToZMKey = Qt::Key_8;
	Qt::Key orthographicCameraToZPKey = Qt::Key_2;
	Qt::Key translateEntityKey = Qt::Key_T;
	Qt::Key rotateEntityKey = Qt::Key_R;
	Qt::Key scaleEntityKey = Qt::Key_E;
	Qt::Key toggleCurrentEntityVisibilityKey = Qt::Key_V;
	Qt::Key toggleGridVisibilityKey = Qt::Key_G;
	Qt::Key toggleBackfaceCullingKey = Qt::Key_F;
	Qt::Key toggleCamerasVisibilityKey = Qt::Key_C;
	Qt::Key toggleLightingKey = Qt::Key_L;
	Qt::Key toggleCollidersVisibilityKey = Qt::Key_X;

	float cameraNearPlane = 0.01f;
	float cameraFarPlane = 500.0f;
	float perspectiveCameraSpeed = 2.0f;
	float orthographicCameraSpeed = 2.0f;
	float cameraSensitivity = 0.5f;

	float gridScale = 10.0f;

	bool maintainGuizmoSize = false;
	float guizmoSize = 1.0f;
	nml::vec3 guizmoTranslationStep = nml::vec3(0.25f, 0.25f, 0.25f);
	nml::vec3 guizmoRotationStep = nml::vec3(10.0f, 10.0f, 10.0f);
	nml::vec3 guizmoScaleStep = nml::vec3(0.1f, 0.1f, 0.1f);

	nml::vec3 currentEntityOutlineColor = nml::vec3(1.0f, 1.0f, 0.0f);
	nml::vec3 otherEntitiesOutlineColor = nml::vec3(1.0f, 0.7f, 0.0f);

	bool showGrid = true;
	bool enableBackfaceCulling = false;
	bool showCameras = false;
	bool enableLighting = false;
	bool showColliders = false;
};

struct BuildParameters {
	std::string cMakePath = "cmake";
};

struct CodeParameters {
#if defined(NTSHENGN_OS_WINDOWS)
	std::string codeEditorCommand = "notepad ${FILE_PATH}";
#elif defined(NTSHENGN_OS_LINUX) || defined(NTSHENGN_OS_FREEBSD)
	std::string codeEditorCommand = "nano ${FILE_PATH}";
#endif
};

class EditorParameters {
public:
	EditorParameters();

	void fromJson(const nlohmann::json& j);
	nlohmann::json toJson() const;

public:
	RendererParameters renderer = {};
	BuildParameters build = {};
	CodeParameters code = {};
};