#pragma once
#include "../../utils/ntshengn_utils_math.h"
#include <array>

namespace NtshEngn {

	enum class LightType {
		Directional,
		Point,
		Spot,
		Ambient,
		Unknown
	};

	struct Light {
		LightType type = LightType::Unknown;
		Math::vec3 color = Math::vec3(0.0f, 0.0f, 0.0f);
		Math::vec3 direction = Math::vec3(0.0f, -1.0f, 0.0f);
		Math::vec2 cutoff = Math::vec2(0.0f, 0.0f);
	};

}