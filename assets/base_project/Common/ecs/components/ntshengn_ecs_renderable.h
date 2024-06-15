#pragma once
#include "../../resources/ntshengn_resources_graphics.h"

namespace NtshEngn {

	struct Renderable {
		Mesh* mesh = nullptr;
		Material material;
	};

}