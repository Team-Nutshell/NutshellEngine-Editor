#pragma once
#include "component.h"
#include "../../external/nml/include/nml.h"

struct Camera : public Component {
	Camera() : Component(ComponentType::Camera) {}

	nml::vec3 forward = nml::vec3(0.0f, 0.0f, -1.0f);
	nml::vec3 up = nml::vec3(0.0f, 1.0f, 0.0f);
	std::string projectionType = "Perspective";
	float fov = 45.0f;
	float left = -10.0f;
	float right = 10.0f;
	float bottom = -10.0f;
	float top = 10.0f;
	float nearPlane = 0.3f;
	float farPlane = 200.0f;

	nlohmann::json toJson() const {
		nlohmann::json j;
		j["forward"] = { forward.x, forward.y, forward.z };
		j["up"] = { up.x, up.y, up.z };
		j["projectionType"] = projectionType;
		if (projectionType == "Perspective") {
			j["fov"] = fov;
		}
		if (projectionType == "Orthographic") {
			j["left"] = left;
			j["right"] = right;
			j["bottom"] = bottom;
			j["top"] = top;
		}
		j["nearPlane"] = nearPlane;
		j["farPlane"] = farPlane;

		return j;
	}

	static Camera fromJson(const nlohmann::json& j) {
		Camera camera;
		if (j.contains("forward")) {
			camera.forward.x = j["forward"][0];
			camera.forward.y = j["forward"][1];
			camera.forward.z = j["forward"][2];
		}
		if (j.contains("up")) {
			camera.up.x = j["up"][0];
			camera.up.y = j["up"][1];
			camera.up.z = j["up"][2];
		}
		if (j.contains("projectionType")) {
			camera.projectionType = j["projectionType"];
		}
		if (j.contains("fov")) {
			camera.fov = j["fov"];
		}
		if (j.contains("left")) {
			camera.left = j["left"];
		}
		if (j.contains("right")) {
			camera.right = j["right"];
		}
		if (j.contains("bottom")) {
			camera.bottom = j["bottom"];
		}
		if (j.contains("top")) {
			camera.top = j["top"];
		}
		if (j.contains("nearPlane")) {
			camera.nearPlane = j["nearPlane"];
		}
		if (j.contains("farPlane")) {
			camera.farPlane = j["farPlane"];
		}

		return camera;
	}
};