#include "localization.h"
#include <fstream>

Localization::Localization() {
	std::fstream languageFile("assets/languages/en.json", std::ios::in);
	if (languageFile.is_open()) {
		if (!nlohmann::json::accept(languageFile)) {
			return;
		}
	}
	else {
		return;
	}

	languageFile = std::fstream("assets/languages/en.json", std::ios::in);
	j = nlohmann::json::parse(languageFile);
}

std::string Localization::getString(const std::string& key, std::vector<std::string> parameters) {
	std::string value = key;
	if (j.contains(key)) {
		value = j[key];
		for (size_t i = 0; i < parameters.size(); i++) {
			std::string sourceString = "${" + std::to_string(i) + "}";
			size_t sourceStringPos;
			while ((sourceStringPos = value.find(sourceString)) != std::string::npos) {
				value.replace(sourceStringPos, sourceString.length(), parameters[i]);
			}
		}
	}

	return value;
}
