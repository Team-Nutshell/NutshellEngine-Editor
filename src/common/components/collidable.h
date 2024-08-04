#pragma once
#include "component.h"
#include "../../external/nml/include/nml.h"
#include <string>

struct Collidable : public Component {
	Collidable() : Component(ComponentType::Collidable) {}

	std::string type = "Box";
	nml::vec3 center = nml::vec3(0.0f, 0.0f, 0.0f);
	float radius = 0.5f;
	nml::vec3 halfExtent = nml::vec3(0.5f, 0.5f, 0.5f);
	nml::vec3 rotation = nml::vec3(0.0f, 0.0f, 0.0f);
	nml::vec3 base = nml::vec3(0.0f, 0.0f, 0.0f);
	nml::vec3 tip = nml::vec3(0.0f, 0.5f, 0.0f);

	nlohmann::json toJson() const {
		nlohmann::json j;
		j["type"] = type;
		if ((type == "Box") || (type == "Sphere")) {
			j["center"] = { center.x, center.y, center.z };
		}
		if ((type == "Sphere") || (type == "Capsule")) {
			j["radius"] = radius;
		}
		if (type == "Box") {
			j["halfExtent"] = { halfExtent.x, halfExtent.y, halfExtent.z };
			j["rotation"] = { rotation.x, rotation.y, rotation.z };
		}
		if (type == "Capsule") {
			j["base"] = { base.x, base.y, base.z };
			j["tip"] = { tip.x, tip.y, tip.z };
		}

		return j;
	}

	static Collidable fromJson(const nlohmann::json& j) {
		Collidable collidable;
		if (j.contains("type")) {
			collidable.type = j["type"];
		}
		if (j.contains("center")) {
			collidable.center.x = j["center"][0];
			collidable.center.y = j["center"][1];
			collidable.center.z = j["center"][2];
		}
		if (j.contains("radius")) {
			collidable.radius = j["radius"];
		}
		if (j.contains("halfExtent")) {
			collidable.halfExtent.x = j["halfExtent"][0];
			collidable.halfExtent.y = j["halfExtent"][1];
			collidable.halfExtent.z = j["halfExtent"][2];
		}
		if (j.contains("rotation")) {
			collidable.rotation.x = j["rotation"][0];
			collidable.rotation.y = j["rotation"][1];
			collidable.rotation.z = j["rotation"][2];
		}
		if (j.contains("base")) {
			collidable.base.x = j["base"][0];
			collidable.base.y = j["base"][1];
			collidable.base.z = j["base"][2];
		}
		if (j.contains("tip")) {
			collidable.tip.x = j["tip"][0];
			collidable.tip.y = j["tip"][1];
			collidable.tip.z = j["tip"][2];
		}

		return collidable;
	}
};