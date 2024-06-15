#pragma once
#include "../utils/ntshengn_defines.h"
#include "../utils/ntshengn_utils_math.h"
#include <vector>
#include <memory>

#define NTSHENGN_SCRIPT(scriptName)

namespace NtshEngn {

	// Collision info
	struct CollisionInfo {
		using Entity = uint32_t;

		Entity otherEntity = 0xFFFFFFFF;
		Math::vec3 normal;
		float depth = 0.0f;
		std::vector<Math::vec3> relativePoints;
	};

	// Script
	struct ScriptBase {
		virtual void onCollisionEnter(CollisionInfo collisionInfo) {
			NTSHENGN_UNUSED(collisionInfo);
		}
		virtual void onCollisionStill(CollisionInfo collisionInfo) {
			NTSHENGN_UNUSED(collisionInfo);
		}
		virtual void onCollisionExit(CollisionInfo collisionInfo) {
			NTSHENGN_UNUSED(collisionInfo);
		}
	};

}