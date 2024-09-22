#pragma once
#include "../../external/nlohmann/json.hpp"

enum class ComponentType {
	Camera,
	Collidable,
	Light,
	Renderable,
	Rigidbody,
	Scriptable,
	Transform,
	Unknown
};

struct Component {
	Component(ComponentType type) : componentType(type) {}
	virtual ~Component() {}

	ComponentType componentType = ComponentType::Unknown;

	virtual nlohmann::json toJson() const = 0;
};