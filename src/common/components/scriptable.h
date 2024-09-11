#pragma once
#include "component.h"
#include <string>
#include <utility>
#include <variant>
#include <unordered_map>

typedef std::variant<bool, int, double, std::string, nml::vec2, nml::vec3, nml::vec4, nml::quat> EditableScriptVariableValue;

struct Scriptable : public Component {
	Scriptable() : Component(ComponentType::Scriptable) {}

	std::string scriptName = "";
	std::unordered_map<std::string, std::pair<std::string, EditableScriptVariableValue>> editableVariables;

	nlohmann::json toJson() const {
		nlohmann::json j;
		j["scriptName"] = scriptName;

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