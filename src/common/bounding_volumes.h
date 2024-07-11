#pragma once
#include "../../external/nml/include/nml.h"
#include <limits>

struct AABB {
	nml::vec3 min = nml::vec3(std::numeric_limits<float>::max());
	nml::vec3 max = nml::vec3(std::numeric_limits<float>::lowest());
};

struct OBB {
	nml::vec3 center;
	nml::vec3 halfExtent;
	nml::vec3 rotation;
};