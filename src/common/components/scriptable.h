#pragma once
#include "component.h"
#include <string>

struct Scriptable : public Component {
	Scriptable() : Component(ComponentType::Scriptable) {}

	std::string scriptName = "";

	nlohmann::json toJson() const {
		nlohmann::json j;
		if (scriptName != "") {
			j["scriptName"] = scriptName;
		}

		return j;
	}

	static Scriptable fromJson(const nlohmann::json& j) {
		Scriptable scriptable;
		if (j.contains("scriptName")) {
			scriptable.scriptName = j["scriptName"];
		}

		return scriptable;
	}
};