#pragma once
#include "component.h"

struct SoundListener : public Component {
	SoundListener() : Component(ComponentType::SoundListener) {}

	nml::vec3 forward = nml::vec3(0.0f, 0.0f, -1.0f);
	nml::vec3 up = nml::vec3(0.0f, 1.0f, 0.0f);

	nlohmann::json toJson() const {
		nlohmann::json j;
		j["forward"] = { forward.x, forward.y, forward.z };
		j["up"] = { up.x, up.y, up.z };

		return j;
	}

	static SoundListener fromJson(const nlohmann::json& j) {
		SoundListener soundListener;
		if (j.contains("forward")) {
			soundListener.forward.x = j["forward"][0];
			soundListener.forward.y = j["forward"][1];
			soundListener.forward.z = j["forward"][2];
		}
		if (j.contains("up")) {
			soundListener.up.x = j["up"][0];
			soundListener.up.y = j["up"][1];
			soundListener.up.z = j["up"][2];
		}

		return soundListener;
	}
};