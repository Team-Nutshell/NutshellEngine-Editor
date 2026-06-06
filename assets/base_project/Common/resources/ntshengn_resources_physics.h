#pragma once
#include "../utils/ntshengn_utils_math.h"
#include <variant>
#include <utility>
#include <vector>

namespace NtshEngn {

	struct ColliderBox {
		Math::vec3 center;
		Math::vec3 halfExtent;
		Math::vec3 rotation;
	};

	struct ColliderSphere {
		Math::vec3 center;
		float radius = 0.0f;
	};

	struct ColliderCapsule {
		Math::vec3 base;
		Math::vec3 tip;
		float radius = 0.0f;
	};

	typedef std::variant<ColliderBox, ColliderSphere, ColliderCapsule> ColliderShape;

	struct IntersectionInformation {
		bool hasIntersected = false;
		Math::vec3 normal;
		float depth = 0.0f;
		std::vector<std::pair<Math::vec3, Math::vec3>> relativePoints;
	};

	// Raycast
	struct RaycastInformation {
		bool hasIntersected = false;
		Math::vec3 normal;
		float distance = 0.0f;
	};

}