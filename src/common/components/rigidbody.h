#pragma once
#include "component.h"

struct Rigidbody : public Component {
	Rigidbody() : Component(ComponentType::Rigidbody) {}

	bool isStatic = false;
	bool isAffectedByConstants = true;
	bool lockRotation = false;
	float mass = 1.0f;
	float inertia = 1.0f;
	float restitution = 0.0f;
	float staticFriction = 0.0f;
	float dynamicFriction = 0.0f;

	nlohmann::json toJson() const {
		nlohmann::json j;
		j["isStatic"] = isStatic;
		j["isAffectedByConstants"] = isAffectedByConstants;
		j["lockRotation"] = lockRotation;
		j["mass"] = mass;
		j["inertia"] = inertia;
		j["restitution"] = restitution;
		j["staticFriction"] = staticFriction;
		j["dynamicFriction"] = dynamicFriction;

		return j;
	}

	static Rigidbody fromJson(const nlohmann::json& j) {
		Rigidbody rigidbody;
		if (j.contains("isStatic")) {
			rigidbody.isStatic = j["isStatic"];
		}
		if (j.contains("isAffectedByConstants")) {
			rigidbody.isAffectedByConstants = j["isAffectedByConstants"];
		}
		if (j.contains("lockRotation")) {
			rigidbody.lockRotation = j["lockRotation"];
		}
		if (j.contains("mass")) {
			rigidbody.mass = j["mass"];
		}
		if (j.contains("inertia")) {
			rigidbody.inertia = j["inertia"];
		}
		if (j.contains("restitution")) {
			rigidbody.restitution = j["restitution"];
		}
		if (j.contains("staticFriction")) {
			rigidbody.staticFriction = j["staticFriction"];
		}
		if (j.contains("dynamicFriction")) {
			rigidbody.dynamicFriction = j["dynamicFriction"];
		}

		return rigidbody;
	}
};