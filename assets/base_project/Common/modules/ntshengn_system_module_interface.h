#pragma once
#include "ntshengn_module_interface.h"
#include "../ecs/ntshengn_ecs_interface.h"
#include "../asset_manager/ntshengn_asset_manager_interface.h"
#include "../job_system/ntshengn_job_system_interface.h"
#include <string>

namespace NtshEngn {

	class GraphicsModuleInterface;
	class PhysicsModuleInterface;
	class WindowModuleInterface;
	class AudioModuleInterface;
	class PlatformModuleInterface;

	class SystemModuleInterface : public ModuleInterface, public System {
	public:
		SystemModuleInterface() {}
		SystemModuleInterface(const ModuleType type, const std::string& name) : ModuleInterface(type, name) {}
		virtual ~SystemModuleInterface() {}

		virtual const ComponentMask getComponentMask() const {
			return ComponentMask();
		}

		void setModules(GraphicsModuleInterface* passGraphicsModule, PhysicsModuleInterface* passPhysicsModule, WindowModuleInterface* passWindowModule, AudioModuleInterface* passAudioModule, PlatformModuleInterface* passPlatformModule) {
			graphicsModule = passGraphicsModule;
			physicsModule = passPhysicsModule;
			windowModule = passWindowModule;
			audioModule = passAudioModule;
			platformModule = passPlatformModule;
		}

		void setECS(ECSInterface* passECS) {
			ecs = passECS;
		}

		void setAssetManager(AssetManagerInterface* passAssetManager) {
			assetManager = passAssetManager;
		}

		void setJobSystem(JobSystemInterface* passJobSystem) {
			jobSystem = passJobSystem;
		}
		
	protected:
		GraphicsModuleInterface* graphicsModule = nullptr;
		PhysicsModuleInterface* physicsModule = nullptr;
		WindowModuleInterface* windowModule = nullptr;
		AudioModuleInterface* audioModule = nullptr;
		PlatformModuleInterface* platformModule = nullptr;

		ECSInterface* ecs = nullptr;

		AssetManagerInterface* assetManager = nullptr;

		JobSystemInterface* jobSystem = nullptr;
	};

}