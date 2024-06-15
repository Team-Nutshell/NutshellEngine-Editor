#pragma once
#include "../../utils/ntshengn_utils_math.h"
#include <array>

namespace NtshEngn {

	struct Rigidbody {
		bool isStatic = false;
		bool isAffectedByConstants = true;
		bool lockRotation = false;
		float mass = 1.0f;
		float inertia = 1.0f;
		float restitution = 0.0f;
		float staticFriction = 0.0f;
		float dynamicFriction = 0.0f;
		
		Math::vec3 force = Math::vec3(0.0f, 0.0f, 0.0f);
		Math::vec3 linearAcceleration = Math::vec3(0.0f, 0.0f, 0.0f);
		Math::vec3 linearVelocity = Math::vec3(0.0f, 0.0f, 0.0f);

		Math::vec3 torque = Math::vec3(0.0f, 0.0f, 0.0f);
		Math::vec3 angularAcceleration = Math::vec3(0.0f, 0.0f, 0.0f);
		Math::vec3 angularVelocity = Math::vec3(0.0f, 0.0f, 0.0f);
	};

}