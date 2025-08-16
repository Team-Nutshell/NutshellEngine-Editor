#pragma once
#include "ntshengn_module_interface.h"
#include "../resources/ntshengn_resources_platform.h"
#include <vector>

namespace NtshEngn {

	class PlatformModuleInterface : public ModuleInterface {
	public:
		PlatformModuleInterface() {}
		PlatformModuleInterface(const std::string& name) : ModuleInterface(ModuleType::Platform, name) {}
		virtual ~PlatformModuleInterface() {}

		// Returns the user's information on the platform
		virtual PlatformUserInfo getUserInfo() = 0;

		// Returns the list of the user's friends on the platform
		virtual std::vector<PlatformUserInfo> getUserFriendsInfo() = 0;

		// Unlocks achievement with identifier achievementID
		virtual void unlockAchievement(const std::string& achievementID) = 0;
		// Locks achievement with identifier achievementID
		virtual void lockAchievement(const std::string& achievementID) = 0;
		// Returns true if the achievement with identifier achievementID has been unlocked by the user, else, returns false
		virtual bool isAchievementUnlocked(const std::string& achievementID) = 0;

		// Show the platform's overlay
		virtual void showOverlay() = 0;
		// Returns true if the platform's overlay is visible, else, returns false
		virtual bool isOverlayVisible() = 0;
	};

}