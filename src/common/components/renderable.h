#pragma once
#include "component.h"
#include <string>

#define NTSHENGN_NO_MODEL_PRIMITIVE 0xFFFFFFFF

struct Renderable : public Component {
	Renderable() : Component(ComponentType::Renderable) {}
	
	std::string modelPath = "";
	uint32_t primitiveIndex = NTSHENGN_NO_MODEL_PRIMITIVE;
	std::string materialPath = "";

	nlohmann::json toJson() const {
		nlohmann::json j;
		if (!modelPath.empty()) {
			j["modelPath"] = modelPath;
		}
		if (primitiveIndex != NTSHENGN_NO_MODEL_PRIMITIVE) {
			j["primitiveIndex"] = primitiveIndex;
		}
		if (!materialPath.empty()) {
			j["materialPath"] = materialPath;
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
		if (j.contains("materialPath")) {
			renderable.materialPath = j["materialPath"];
		}

		return renderable;
	}
};