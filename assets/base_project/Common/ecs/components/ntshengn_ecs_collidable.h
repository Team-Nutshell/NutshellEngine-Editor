#pragma once
#include "../../resources/ntshengn_resources_physics.h"
#include <memory>

namespace NtshEngn {

	struct Collidable {
		Collidable() = default;
		~Collidable() = default;
		Collidable(const Collidable& other) : collider(other.collider->clone()) {}
		Collidable(Collidable&& other) = default;
		Collidable& operator=(const Collidable& other) { collider = other.collider->clone(); return *this; }
		Collidable& operator=(Collidable&& other) = default;

		std::unique_ptr<ColliderShape> collider;
	};

}