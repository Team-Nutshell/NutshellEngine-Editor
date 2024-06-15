#pragma once
#include <string>
#include <algorithm>
#include <limits>
#include <set>
#include <vector>

namespace NtshEngn {

	class SceneManagerInterface {
	public:
		virtual void goToScene(const std::string& filePath) = 0;

		virtual std::string getCurrentScenePath() = 0;
	};

}