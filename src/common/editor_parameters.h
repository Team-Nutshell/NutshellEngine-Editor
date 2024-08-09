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
	Qt::Key cameraDownKey = Qt::Key_Shift;
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
	Qt::Key multiSelectionKey = Qt::Key_Z;

	float cameraSpeed = 2.0f;
	float cameraSensitivity = 0.5f;

	nml::vec3 currentEntityOutlineColor = nml::vec3(1.0f, 1.0f, 0.0f);
	nml::vec3 otherEntitiesOutlineColor = nml::vec3(1.0f, 0.7f, 0.0f);
};

struct BuildParameters {
	std::string cMakePath = "cmake";
};

struct CodeParameters {
#if NTSHENGN_OS_WINDOWS
	std::string codeEditorCommand = "notepad ${FILE_PATH}";
#elif NTSHENGN_OS_LINUX
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