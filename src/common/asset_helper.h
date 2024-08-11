#pragma once
#include <string>
#include <algorithm>
#include <filesystem>

struct AssetHelper {
	static std::string absoluteToRelative(std::string path, const std::string& projectDirectory) {
		if (!path.empty()) {
			std::replace(path.begin(), path.end(), '\\', '/');
			if (projectDirectory != "") {
				if (std::filesystem::path(path).is_absolute()) {
					if (path.substr(0, projectDirectory.size()) == projectDirectory) {
						path = path.substr(projectDirectory.size() + 1);
					}
				}
			}
		}

		return path;
	}
};