#pragma once
#include "../utils/ntshengn_enums.h"
#include "../profiler/ntshengn_profiler_interface.h"
#include <string>

namespace NtshEngn {

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

		void setProfiler(ProfilerInterface* passProfiler) {
			profiler = passProfiler;
		}

	protected:
		ProfilerInterface* profiler = nullptr;

	private:
		ModuleType m_type = ModuleType::Unknown;
		std::string m_name = "";
	};

}