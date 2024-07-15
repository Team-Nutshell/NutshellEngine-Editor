#pragma once
#include "../../external/nml/include/nml.h"
#include <cmath>

struct RendererCamera {
	RendererCamera() {
		perspectiveYaw = nml::toDeg(std::atan2(perspectiveDirection[2], perspectiveDirection[0]));
		perspectivePitch = nml::toDeg(-std::asin(perspectiveDirection[1]));

		basePerspectivePosition = perspectivePosition;
		basePerspectiveDirection = perspectiveDirection;

		baseOrthographicPosition = orthographicPosition;
		baseOrthographicDirection = orthographicDirection;
		baseOrthographicUp = orthographicUp;
		baseOrthographicHalfExtent = orthographicHalfExtent;
	}

	nml::vec3 perspectivePosition = nml::vec3(0.0f, 1.0f, 1.0f);
	nml::vec3 perspectiveDirection = nml::normalize(nml::vec3(0.0f, -1.0f, -1.0f));
	nml::vec3 perspectiveUp = nml::vec3(0.0f, 1.0f, 0.0f);

	float perspectiveYaw;
	float perspectivePitch;

	nml::vec3 orthographicPosition = nml::vec3(0.0f, 0.0f, 0.0f);
	nml::vec3 orthographicDirection = nml::vec3(0.0f, 0.0f, -1.0f);
	nml::vec3 orthographicUp = nml::vec3(0.0f, 1.0f, 0.0f);
	float orthographicHalfExtent = 10.0f;

	float nearPlane = 0.01f;
	float farPlane = 500.0f;

	bool useOrthographicProjection = false;

	nml::mat4 viewMatrix;
	nml::mat4 projectionMatrix;
	nml::mat4 viewProjMatrix;
	nml::mat4 invViewMatrix;
	nml::mat4 invProjMatrix;

	nml::vec3 basePerspectivePosition;
	nml::vec3 basePerspectiveDirection;

	nml::vec3 baseOrthographicPosition;
	nml::vec3 baseOrthographicDirection;
	nml::vec3 baseOrthographicUp;
	float baseOrthographicHalfExtent;

	float speed = 2.0f;

	float sensitivity = 0.5f;
};