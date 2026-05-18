#pragma once
#include "ntshengn_module_interface.h"
#include "../ecs/ntshengn_ecs_interface.h"
#include <string>

namespace NtshEngn {

	class SystemModuleInterface : public ModuleInterface, public System {
	public:
		SystemModuleInterface() {}
		SystemModuleInterface(const ModuleType type, const std::string& name) : ModuleInterface(type, name) {}
		virtual ~SystemModuleInterface() {}

		virtual const ComponentMask getComponentMask() const {
			return ComponentMask();
		}

		void setECS(ECSInterface* passECS) {
			ecs = passECS;
		}
		
	protected:
		ECSInterface* ecs = nullptr;
	};

}