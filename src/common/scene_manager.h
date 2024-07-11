#pragma once
#include "global_info.h"
#include <string>

struct SceneManager {
	static void newScene(GlobalInfo& globalInfo);
	static void openScene(GlobalInfo& globalInfo, const std::string& sceneFilePath);
	static void saveScene(GlobalInfo& globalInfo, const std::string& sceneFilePath);
};