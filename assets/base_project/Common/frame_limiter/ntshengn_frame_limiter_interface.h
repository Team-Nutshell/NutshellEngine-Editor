#pragma once
#include <cstdint>

namespace NtshEngn {

	class FrameLimiterInterface {
	public:
		virtual void wait(double frameStart) = 0;

		virtual void setMaxFPS(uint32_t maxFPS) = 0;
		virtual uint32_t getMaxFPS() = 0;
	};

}