#pragma once
#include "../utils/ntshengn_enums.h"
#include "../asset_manager/ntshengn_asset_manager_interface.h"
#include "../job_system/ntshengn_job_system_interface.h"
#include "../command_line/ntshengn_command_line_interface.h"
#include "../profiler/ntshengn_profiler_interface.h"
#include <string>

namespace NtshEngn {

	class GraphicsModuleInterface;
	class PhysicsModuleInterface;
	class WindowModuleInterface;
	class AudioModuleInterface;
	class PlatformModuleInterface;

	class ModuleInterface {
	public:
		ModuleInterface() {}
		ModuleInterface(const ModuleType type, const std::string& name) : m_type(type), m_name(name) {}
		virtual ~ModuleInterface() {}

		virtual void init() = 0;
		virtual void update(float dt) = 0;
		virtual void destroy() = 0;

		const ModuleType getType() const {
			return m_type;
		}

		const std::string& getName() const {
			return m_name;
		}

		void setModules(GraphicsModuleInterface* passGraphicsModule, PhysicsModuleInterface* passPhysicsModule, WindowModuleInterface* passWindowModule, AudioModuleInterface* passAudioModule, PlatformModuleInterface* passPlatformModule) {
			graphicsModule = passGraphicsModule;
			physicsModule = passPhysicsModule;
			windowModule = passWindowModule;
			audioModule = passAudioModule;
			platformModule = passPlatformModule;
		}

		void setAssetManager(AssetManagerInterface* passAssetManager) {
			assetManager = passAssetManager;
		}

		void setJobSystem(JobSystemInterface* passJobSystem) {
			jobSystem = passJobSystem;
		}

		void setCommandLine(CommandLineInterface* passCommandLine) {
			commandLine = passCommandLine;
		}

		void setProfiler(ProfilerInterface* passProfiler) {
			profiler = passProfiler;
		}

	protected:
		GraphicsModuleInterface* graphicsModule = nullptr;
		PhysicsModuleInterface* physicsModule = nullptr;
		WindowModuleInterface* windowModule = nullptr;
		AudioModuleInterface* audioModule = nullptr;
		PlatformModuleInterface* platformModule = nullptr;

		AssetManagerInterface* assetManager = nullptr;
		JobSystemInterface* jobSystem = nullptr;
		CommandLineInterface* commandLine = nullptr;
		ProfilerInterface* profiler = nullptr;

	private:
		ModuleType m_type = ModuleType::Unknown;
		std::string m_name = "";
	};

}