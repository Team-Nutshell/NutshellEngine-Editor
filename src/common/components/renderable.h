#pragma once
#include "component.h"
#include <string>

#define NTSHENGN_NO_MODEL_PRIMITIVE 0xFFFFFFFF

struct Renderable : public Component {
	Renderable() : Component(ComponentType::Renderable) {}
	
	std::string modelPath = "";
	uint32_t primitiveIndex = NTSHENGN_NO_MODEL_PRIMITIVE;
	std::string materialPath = "";
	std::string fragmentShaderPath = "";
	bool isVisible = true;
	bool castsShadows = true;

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
		if (!fragmentShaderPath.empty()) {
			j["fragmentShaderPath"] = fragmentShaderPath;
		}
		j["isVisible"] = isVisible;
		j["castsShadows"] = castsShadows;

		if (j.empty()) {
			j = nlohmann::json::object();
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
		if (j.contains("fragmentShaderPath")) {
			renderable.fragmentShaderPath = j["fragmentShaderPath"];
		}
		if (j.contains("isVisible")) {
			renderable.isVisible = j["isVisible"];
		}
		if (j.contains("castsShadows")) {
			renderable.castsShadows = j["castsShadows"];
		}

		return renderable;
	}
};