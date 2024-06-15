#pragma once
#include <cstdint>
#include <cstddef>
#include <vector>

namespace NtshEngn {

	// Sound
	typedef uint32_t SoundID;
	#define NTSHENGN_SOUND_UNKNOWN 0xFFFFFFFF

	typedef uint32_t SoundSourceID;
	#define NTSHENGN_SOUND_SOURCE_UNKNOWN 0xFFFFFFFF

	struct Sound {
		// Number of channels (1 = Mono, 2 = Stereo, ...)
		uint8_t channels = 0;

		// Sample rate
		int32_t sampleRate = 0;

		// Number of bits per sample
		uint8_t bitsPerSample = 0;

		// Size of the data
		size_t size = 0;

		// Data
		std::vector<uint8_t> data;
	};

	enum class SoundSourceState {
		Playing,
		Paused,
		Stopped
	};

}