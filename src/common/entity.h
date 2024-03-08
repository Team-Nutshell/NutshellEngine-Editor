#pragma once
#include "components/component.h"
#include "components/camera.h"
#include "components/collidable.h"
#include "components/component.h "
#include "components/light.h"
#include "components/renderable.h"
#include "components/rigidbody.h"
#include "components/scriptable.h"
#include "components/transform.h"
#include "../../external/nlohmann/json.hpp"
#include <string>
#include <map>
#include <memory>
#include <optional>
#include <cstdint>

typedef uint32_t EntityID;
#define NO_ENTITY 0xFFFFFFFF

struct Entity {
	EntityID entityID = NO_ENTITY;
	std::string name = "";
	bool isPersistent = false;

	Transform transform = {};
	std::optional<Camera> camera;
	std::optional<Light> light;
	std::optional<Renderable> renderable;
	std::optional<Rigidbody> rigidbody;
	std::optional<Collidable> collidable;
	std::optional<Scriptable> scriptable;

	bool isVisible = true;

	nlohmann::json toJson() {
		nlohmann::json j;
		j["name"] = name;
		j["isPersistent"] = isPersistent;
		j["transform"] = transform.toJson();
		if (camera) {
			j["camera"] = camera->toJson();
		}
		if (light) {
			j["light"] = light->toJson();
		}
		if (renderable) {
			j["renderable"] = renderable->toJson();
		}
		if (rigidbody) {
			j["rigidbody"] = rigidbody->toJson();
		}
		if (collidable) {
			j["collidable"] = collidable->toJson();
		}
		if (scriptable) {
			j["scriptable"] = scriptable->toJson();
		}

		return j;
	}

	static Entity fromJson(const nlohmann::json& j) {
		Entity entity;
		if (j.contains("name")) {
			entity.name = j["name"];
		}
		if (j.contains("isPersistent")) {
			entity.isPersistent = j["isPersistent"];
		}
		if (j.contains("transform")) {
			entity.transform = Transform::fromJson(j["transform"]);
		}
		if (j.contains("camera")) {
			entity.camera = Camera::fromJson(j["camera"]);
		}
		if (j.contains("light")) {
			entity.light = Light::fromJson(j["light"]);
		}
		if (j.contains("renderable")) {
			entity.renderable = Renderable::fromJson(j["renderable"]);
		}
		if (j.contains("rigidbody")) {
			entity.rigidbody = Rigidbody::fromJson(j["rigidbody"]);
		}
		if (j.contains("collidable")) {
			entity.collidable = Collidable::fromJson(j["collidable"]);
		}
		if (j.contains("scriptable")) {
			entity.scriptable = Scriptable::fromJson(j["scriptable"]);
		}

		return entity;
	}
};