#pragma once
#include <vector>
#include <string>
#include "../../external/nlohmann/json.hpp"

class Localization {
public:
	Localization();

	std::string getString(const std::string& key, std::vector<std::string> parameters = std::vector<std::string>());

private:
	nlohmann::json j;
};