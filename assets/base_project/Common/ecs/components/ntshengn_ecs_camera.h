#pragma once

namespace NtshEngn {

	struct Camera {
		Math::vec3 forward = Math::vec3(0.0f, 0.0f, 1.0f);
		Math::vec3 up = Math::vec3(0.0f, 1.0f, 0.0f);
		float fov = 45.0f;
		float nearPlane = 0.3f;
		float farPlane = 200.0f;
	};

}