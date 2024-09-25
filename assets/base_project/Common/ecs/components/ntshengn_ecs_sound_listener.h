#pragma once
#include "../../utils/ntshengn_utils_math.h"

namespace NtshEngn {

	struct SoundListener {
		Math::vec3 forward = Math::vec3(0.0f, 0.0f, 1.0f);
		Math::vec3 up = Math::vec3(0.0f, 1.0f, 0.0f);
	};

}