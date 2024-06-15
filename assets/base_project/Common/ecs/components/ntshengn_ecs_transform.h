#pragma once
#include "../../utils/ntshengn_utils_math.h"
#include <array>

namespace NtshEngn {

	struct Transform {
		Math::vec3 position = Math::vec3(0.0f, 0.0f, 0.0f);
		Math::vec3 rotation = Math::vec3(0.0f, 0.0f, 0.0f);
		Math::vec3 scale = Math::vec3(1.0f, 1.0f, 1.0f);
	};

}