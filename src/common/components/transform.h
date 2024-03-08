#pragma once
#include "component.h"
#include "../../external/nml/include/nml.h"

struct Transform : public Component {
	Transform() : Component(ComponentType::Transform) {}

	nml::vec3 position = nml::vec3(0.0f, 0.0f, 0.0f);
	nml::vec3 rotation = nml::vec3(0.0f, 0.0f, 0.0f);
	nml::vec3 scale = nml::vec3(1.0f, 1.0f, 1.0f);

	nlohmann::json toJson() const {
		nlohmann::json j;
		j["position"] = { position.x, position.y, position.z };
		j["rotation"] = { rotation.x, rotation.y, rotation.z };
		j["scale"] = { scale.x, scale.y, scale.z };

		return j;
	}

	static Transform fromJson(const nlohmann::json& j) {
		Transform transform;
		if (j.contains("position")) {
			transform.position.x = j["position"][0];
			transform.position.y = j["position"][1];
			transform.position.z = j["position"][2];
		}
		if (j.contains("rotation")) {
			transform.rotation.x = j["rotation"][0];
			transform.rotation.y = j["rotation"][1];
			transform.rotation.z = j["rotation"][2];
		}
		if (j.contains("scale")) {
			transform.scale.x = j["scale"][0];
			transform.scale.y = j["scale"][1];
			transform.scale.z = j["scale"][2];
		}

		return transform;
	}
};