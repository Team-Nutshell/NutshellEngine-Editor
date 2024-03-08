#pragma once
#include "component.h"
#include <string>

struct Renderable : public Component {
	Renderable() : Component(ComponentType::Renderable) {}
	
	std::string modelPath = "";

	nlohmann::json toJson() const {
		nlohmann::json j;
		j["modelPath"] = modelPath;

		return j;
	}

	static Renderable fromJson(const nlohmann::json& j) {
		Renderable renderable;
		if (j.contains("modelPath")) {
			renderable.modelPath = j["modelPath"];
		}

		return renderable;
	}
};