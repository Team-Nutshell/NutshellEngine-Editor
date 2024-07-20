#pragma once
#include <Qt>
#include <string>

struct RendererParameters {
	Qt::Key cameraForwardKey = Qt::Key_W;
	Qt::Key cameraBackwardKey = Qt::Key_S;
	Qt::Key cameraLeftKey = Qt::Key_A;
	Qt::Key cameraRightKey = Qt::Key_D;
	Qt::Key cameraUpKey = Qt::Key_Space;
	Qt::Key cameraDownKey = Qt::Key_Shift;
	Qt::Key switchCameraProjection = Qt::Key_P;
	Qt::Key resetCamera = Qt::Key_0;
	Qt::Key orthographicCameraToXM = Qt::Key_4;
	Qt::Key orthographicCameraToXP = Qt::Key_6;
	Qt::Key orthographicCameraToYM = Qt::Key_1;
	Qt::Key orthographicCameraToYP = Qt::Key_7;
	Qt::Key orthographicCameraToZM = Qt::Key_8;
	Qt::Key orthographicCameraToZP = Qt::Key_2;
	Qt::Key translateEntityKey = Qt::Key_T;
	Qt::Key rotateEntityKey = Qt::Key_R;
	Qt::Key scaleEntityKey = Qt::Key_S;
	Qt::Key toggleCurrentEntityVisibility = Qt::Key_V;
	Qt::Key toggleGridVisibility = Qt::Key_G;
	Qt::Key toggleBackfaceCulling = Qt::Key_F;
	Qt::Key toggleCamerasVisibility = Qt::Key_C;
	Qt::Key toggleLighting = Qt::Key_L;
	Qt::Key toggleCollidersVisibility = Qt::Key_X;
};

struct BuildParameters {
	std::string cMakePath = "cmake";
};

struct CodeParameters {
#if NTSHENGN_OS_WINDOWS
	std::string codeEditorCommand = "notepad ${FILE_PATH}";
#elif NTSHENGN_OS_LINUX
	std::string codeEditorCommand = "";
#endif
};

class EditorParameters {
public:
	EditorParameters();

public:
	RendererParameters renderer = {};
	BuildParameters build = {};
	CodeParameters code = {};
};