#pragma once
#include "../utils/ntshengn_utils_math.h"
#include <utility>
#include <vector>

namespace NtshEngn {

	// Collision
	enum class ColliderShapeType {
		Box,
		Sphere,
		Capsule,
		Unknown
	};

	struct ColliderShape {
		ColliderShape(const ColliderShapeType shapeType) : type(shapeType) {}
		virtual ~ColliderShape() = default;

		ColliderShapeType getType() const { return type; }

		std::unique_ptr<ColliderShape> clone() { return std::unique_ptr<ColliderShape>(cloneImpl()); }

	protected:
		virtual ColliderShape* cloneImpl() = 0;
		
	private:
		ColliderShapeType type = ColliderShapeType::Unknown;
	};

	struct ColliderBox : public ColliderShape {
		ColliderBox() : ColliderShape(ColliderShapeType::Box) {}

		ColliderBox* cloneImpl() { return new ColliderBox(*this); }

		Math::vec3 center;
		Math::vec3 halfExtent;
		Math::vec3 rotation; 
	};

	struct ColliderSphere : public ColliderShape {
		ColliderSphere() : ColliderShape(ColliderShapeType::Sphere) {}

		ColliderSphere* cloneImpl() { return new ColliderSphere(*this); }

		Math::vec3 center;
		float radius = 0.0f;
	};

	struct ColliderCapsule : public ColliderShape {
		ColliderCapsule() : ColliderShape(ColliderShapeType::Capsule) {}

		ColliderCapsule* cloneImpl() { return new ColliderCapsule(*this); }

		Math::vec3 base;
		Math::vec3 tip;
		float radius = 0.0f;
	};

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