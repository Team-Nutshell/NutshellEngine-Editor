#pragma once
#include <string>
#include <cstdint>

namespace NtshEngn {

	enum class PlatformUserState {
		Offline,
		Online,
		Busy,
		Away,
		Unknown
	};

	struct PlatformUserInfo {
		uint64_t id = 0;
		std::string name = "";
		PlatformUserState state = PlatformUserState::Unknown;
	};

}