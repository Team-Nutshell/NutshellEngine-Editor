#pragma once
#include "ntshengn_module_interface.h"

namespace NtshEngn {

	class PlatformModuleInterface : public ModuleInterface {
	public:
		PlatformModuleInterface() {}
		PlatformModuleInterface(const std::string& name) : ModuleInterface(ModuleType::AssetLoader, name) {}
		virtual ~PlatformModuleInterface() {}
	};

}