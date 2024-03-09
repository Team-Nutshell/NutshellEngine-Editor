#pragma once
#include "../../external/nml/include/nml.h"
#include <cmath>

struct RendererCamera {
	nml::vec3 perspectivePosition = nml::vec3(0.0f, 1.0f, 1.0f);
	nml::vec3 perspectiveDirection = nml::normalize(nml::vec3(0.0f, -1.0f, -1.0f));
	nml::vec3 perspectiveUp = nml::vec3(0.0f, 1.0f, 0.0f);

	nml::vec3 perspectiveYaw = nml::toDeg(std::atan2(perspectiveDirection[2], perspectiveDirection[0]));
	nml::vec3 perspectivePitch = nml::toDeg(-std::asin(perspectiveDirection[1]));

	nml::vec3 orthographicPosition = nml::vec3(0.0f, 0.0f, 0.0f);
	nml::vec3 orthographicDirection = nml::vec3(0.0f, 0.0f, -1.0f);
	nml::vec3 orthographicUp = nml::vec3(0.0f, 1.0f, 0.0f);
	float orthographicHalfExtent = 10.0f;

	float nearPlane = 0.01f;
	float farPlane = 500.0f;

	nml::mat4 viewMatrix;
	nml::mat4 projectionMatrix;
	nml::mat4 viewProjMatrix;
	nml::mat4 invViewMatrix;
	nml::mat4 invProjMatrix;

	nml::vec3 basePerspectivePosition = perspectivePosition;
	nml::vec3 basePerspectiveDirection = perspectiveDirection;

	nml::vec3 baseOrthographicPosition = orthographicPosition;
	nml::vec3 baseOrthographicDirection = orthographicDirection;
	nml::vec3 baseOrthographicUp = orthographicUp;
	float baseOrthographicHalfExtent = orthographicHalfExtent;
};