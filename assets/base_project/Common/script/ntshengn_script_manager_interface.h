#pragma once
#include "../ecs/components/ntshengn_ecs_scriptable.h"
#include <set>
#include <memory>

namespace NtshEngn {
	
	class ScriptManagerInterface {
	public:
		virtual ~ScriptManagerInterface() {}

		virtual Scriptable createScriptable(const std::string& name) = 0;

	public:
		template <typename T>
		T* createScript() {
			auto newScript = m_scripts.insert(std::make_unique<T>());

			return static_cast<T*>(newScript.first->get());
		}

		void destroyScript(ScriptBase* script) {
			std::set<std::unique_ptr<ScriptBase>>::iterator it = std::find_if(m_scripts.begin(), m_scripts.end(), [script](const std::unique_ptr<ScriptBase>& other) {
				return script == other.get();
				});

			if (it != m_scripts.end()) {
				m_scripts.erase(it);
			}
		}

	private:
		std::set<std::unique_ptr<ScriptBase>> m_scripts;
	};

}