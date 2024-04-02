#pragma once
#include "../Common/script/ntshengn_script_manager_interface.h"

#if defined(NTSHENGN_OS_WINDOWS)
#if defined(NutshellEngineScriptManager_EXPORTS)
#define NTSHENGN_SCRIPT_MANAGER_API __declspec(dllexport)
#else
#define NTSHENGN_SCRIPT_MANAGER_API __declspec(dllimport)
#endif
#else
#define NTSHENGN_SCRIPT_MANAGER_API
#endif

namespace NtshEngn {

	class ScriptManager : public ScriptManagerInterface {
		Scriptable createScriptable(const std::string& name);
	};

}