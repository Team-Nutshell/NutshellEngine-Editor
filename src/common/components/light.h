#pragma once
#include "component.h"
#include "../../external/nml/include/nml.h"
#include <string>

struct Light : public Component {
	Light() : Component(ComponentType::Light) {}

	std::string type = "Directional";
	nml::vec3 color = nml::vec3(1.0f, 1.0f, 1.0f);
	nml::vec3 direction = nml::vec3(0.0f, -1.0f, 0.0f);
	nml::vec2 cutoff = nml::vec2(10.0f, 20.0f);

	nlohmann::json toJson() const {
		nlohmann::json j;
		j["type"] = type;
		j["color"] = { color.x, color.y, color.z };
		j["direction"] = { direction.x, direction.y, direction.z };
		j["cutoff"] = { cutoff.x, cutoff.y };

		return j;
	}

	static Light fromJson(const nlohmann::json& j) {
		Light light;
		if (j.contains("type")) {
			light.type = j["type"];
		}
		if (j.contains("color")) {
			light.color.x = j["color"][0];
			light.color.y = j["color"][1];
			light.color.z = j["color"][2];
		}
		if (j.contains("direction")) {
			light.direction.x = j["direction"][0];
			light.direction.y = j["direction"][1];
			light.direction.z = j["direction"][2];
		}
		if (j.contains("cutoff")) {
			light.cutoff.x = j["cutoff"][0];
			light.cutoff.y = j["cutoff"][1];
		}

		return light;
	}
};