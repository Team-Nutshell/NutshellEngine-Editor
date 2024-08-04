#pragma once
#include "component.h"
#include <string>

#define NTSHENGN_NO_MODEL_PRIMITIVE 0xFFFFFFFF

struct Renderable : public Component {
	Renderable() : Component(ComponentType::Renderable) {}
	
	std::string modelPath = "";
	uint32_t primitiveIndex = NTSHENGN_NO_MODEL_PRIMITIVE;

	nlohmann::json toJson() const {
		nlohmann::json j;
		j["modelPath"] = modelPath;
		if (primitiveIndex != NTSHENGN_NO_MODEL_PRIMITIVE) {
			j["primitiveIndex"] = primitiveIndex;
		}

		return j;
	}

	static Renderable fromJson(const nlohmann::json& j) {
		Renderable renderable;
		if (j.contains("modelPath")) {
			renderable.modelPath = j["modelPath"];
		}
		if (j.contains("primitiveIndex")) {
			renderable.primitiveIndex = j["primitiveIndex"];
		}

		return renderable;
	}
};