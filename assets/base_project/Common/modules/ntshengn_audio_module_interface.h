#pragma once
#include "ntshengn_system_module_interface.h"
#include "../resources/ntshengn_resources_audio.h"
#include "../utils/ntshengn_utils_math.h"

namespace NtshEngn {

	class AudioModuleInterface : public SystemModuleInterface {
	public:
		AudioModuleInterface() {}
		AudioModuleInterface(const std::string& name) : SystemModuleInterface(ModuleType::Audio, name) {}
		virtual ~AudioModuleInterface() {}

		// Loads the sound described in the sound parameter in the internal format and returns a unique identifier
		virtual SoundID load(const Sound& sound) = 0;

		// Plays the sound with identifier soundID at a certain gain and pitch, looping or not, and returns a unique identifier
		virtual SoundSourceID playSound(SoundID soundID, float gain, float pitch, bool looping) = 0;
		// Plays the sound with identifier soundID at a specific position and at a certain gain and pitch, looping or not, and returns a unique identifier
		virtual SoundSourceID playSoundAtPosition(SoundID soundID, const Math::vec3& position, float gain, float pitch, bool looping) = 0;
		// Pauses the sound with identifier soundSourceID
		virtual void resumeSoundSource(SoundSourceID soundSourceID) = 0;
		// Pauses the sound with identifier soundSourceID
		virtual void pauseSoundSource(SoundSourceID soundSourceID) = 0;
		// Stops the sound with identifier soundSourceID
		virtual void stopSoundSource(SoundSourceID soundSourceID) = 0;

		// Returns the state of the sound source
		virtual SoundSourceState getSoundSourceState(SoundSourceID soundSourceID) = 0;
		// Returns true if the sound with identifier soundID has any sound source currently playing, else, returns false
		virtual bool isSoundPlaying(SoundID soundID) = 0;

		// Sets the position of the sound source with identifieer soundSourceID
		virtual void setSoundSourcePosition(SoundSourceID soundSourceID, const Math::vec3& newPosition) = 0;
		// Gets the position of the sound source with identifier soundSourceID
		virtual Math::vec3 getSoundSourcePosition(SoundSourceID soundSourceID) = 0;

		// Sets the gain of the sound source with identifier soundSourceID
		virtual void setSoundSourceGain(SoundSourceID soundSourceID, float newGain) = 0;
		// Gets the gain of the sound source with identifier soundSourceID
		virtual float getSoundSourceGain(SoundSourceID soundSourceID) = 0;

		// Sets the pitch of the sound source with identifier soundSourceID
		virtual void setSoundSourcePitch(SoundSourceID soundSourceID, float newPitch) = 0;
		// Gets the pitch of the sound source with identifier soundSourceID
		virtual float getSoundSourcePitch(SoundSourceID soundSourceID) = 0;

		// If the looping parameter is true, makes the sound source with identifier soundSourceID looping, else, makes the sound source with identifier soundSourceID not looping
		virtual void setSoundSourceLooping(SoundSourceID soundSourceID, bool looping) = 0;
		// Returns true if the sound source with identifier soundSourceID is looping, else, returns false
		virtual bool isSoundSourceLooping(SoundSourceID soundSourceID) = 0;

		// Sets the master gain
		virtual void setMasterGain(float gain) = 0;
		// Returns the master gain
		virtual float getMasterGain() = 0;
	};

}