#pragma once
#include "../../resources/ntshengn_resources_graphics.h"

namespace NtshEngn {

	struct Renderable {
		Mesh* mesh = nullptr;
		Material material;
		std::string fragmentShader;
		bool isVisible = true;
		bool castsShadows = true;
	};

}