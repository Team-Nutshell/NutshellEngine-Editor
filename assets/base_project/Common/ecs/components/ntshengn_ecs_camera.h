#pragma once
#include "../../resources/ntshengn_resources_graphics.h"
#include "../../utils/ntshengn_utils_math.h"

namespace NtshEngn {

	struct Camera {
		Math::vec3 forward = Math::vec3(0.0f, 0.0f, 1.0f);
		Math::vec3 up = Math::vec3(0.0f, 1.0f, 0.0f);
		CameraProjectionType projectionType = CameraProjectionType::Unknown;
		float fov = 45.0f;
		float left = -10.0f;
		float right = 10.0f;
		float bottom = -10.0f;
		float top = 10.0f;
		float nearPlane = 0.3f;
		float farPlane = 200.0f;
	};

}