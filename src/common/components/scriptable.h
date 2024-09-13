#pragma once
#include "component.h"
#include <string>
#include <utility>
#include <variant>
#include <unordered_map>

typedef std::variant<bool, int, float, std::string, nml::vec2, nml::vec3, nml::vec4> EditableScriptVariableValue;

struct Scriptable : public Component {
	Scriptable() : Component(ComponentType::Scriptable) {}

	std::string scriptName = "";
	std::unordered_map<std::string, EditableScriptVariableValue> editableVariables;

	nlohmann::json toJson() const {
		nlohmann::json j;
		j["scriptName"] = scriptName;
		for (const auto& editableVariable : editableVariables) {
			if (std::holds_alternative<bool>(editableVariable.second)) {
				j["editableVariables"][editableVariable.first] = std::get<bool>(editableVariable.second);
			}
			else if (std::holds_alternative<int>(editableVariable.second)) {
				j["editableVariables"][editableVariable.first] = std::get<int>(editableVariable.second);
			}
			else if (std::holds_alternative<float>(editableVariable.second)) {
				j["editableVariables"][editableVariable.first] = std::get<float>(editableVariable.second);
			}
			else if (std::holds_alternative<std::string>(editableVariable.second)) {
				j["editableVariables"][editableVariable.first] = std::get<std::string>(editableVariable.second);
			}
			else if (std::holds_alternative<nml::vec2>(editableVariable.second)) {
				const nml::vec2& value = std::get<nml::vec2>(editableVariable.second);
				j["editableVariables"][editableVariable.first] = { value.x, value.y };
			}
			else if (std::holds_alternative<nml::vec3>(editableVariable.second)) {
				const nml::vec3& value = std::get<nml::vec3>(editableVariable.second);
				j["editableVariables"][editableVariable.first] = { value.x, value.y, value.z };
			}
			else if (std::holds_alternative<nml::vec4>(editableVariable.second)) {
				const nml::vec4& value = std::get<nml::vec4>(editableVariable.second);
				j["editableVariables"][editableVariable.first] = { value.x, value.y, value.z, value.w };
			}
		}

		return j;
	}

	static Scriptable fromJson(const nlohmann::json& j) {
		Scriptable scriptable;
		if (j.contains("scriptName")) {
			scriptable.scriptName = j["scriptName"];
		}
		if (j.contains("editableVariables")) {
			for (const auto& editableVariable : j["editableVariables"].items()) {
				if (editableVariable.value().type() == nlohmann::json::value_t::boolean) {
					bool value = editableVariable.value();
					scriptable.editableVariables[editableVariable.key()] = value;
				}
				else if ((editableVariable.value().type() == nlohmann::json::value_t::number_integer) ||
					(editableVariable.value().type() == nlohmann::json::value_t::number_unsigned)) {
					int value = editableVariable.value();
					scriptable.editableVariables[editableVariable.key()] = value;
				}
				else if (editableVariable.value().type() == nlohmann::json::value_t::number_float) {
					float value = editableVariable.value();
					scriptable.editableVariables[editableVariable.key()] = value;
				}
				else if (editableVariable.value().type() == nlohmann::json::value_t::string) {
					std::string value = editableVariable.value();
					scriptable.editableVariables[editableVariable.key()] = value;
				}
				else if (editableVariable.value().type() == nlohmann::json::value_t::array) {
					if (editableVariable.value().size() == 2) {
						nml::vec2 value = { editableVariable.value()[0], editableVariable.value()[1] };
						scriptable.editableVariables[editableVariable.key()] = value;
					}
					else if (editableVariable.value().size() == 3) {
						nml::vec3 value = { editableVariable.value()[0], editableVariable.value()[1], editableVariable.value()[2] };
						scriptable.editableVariables[editableVariable.key()] = value;
					}
					else if (editableVariable.value().size() == 4) {
						nml::vec4 value = { editableVariable.value()[0], editableVariable.value()[1], editableVariable.value()[2], editableVariable.value()[3] };
						scriptable.editableVariables[editableVariable.key()] = value;
					}
				}
			}
		}

		return scriptable;
	}
};