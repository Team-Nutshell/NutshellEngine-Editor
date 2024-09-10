#pragma once
#include "../utils/ntshengn_defines.h"
#include "../utils/ntshengn_utils_math.h"
#include <string>
#include <vector>
#include <unordered_map>

#define NTSHENGN_SCRIPT(scriptName) void createEditableScriptVariableMap();
#define NTSHENGN_EDITABLE_VARIABLE

namespace NtshEngn {

	// Collision info
	struct CollisionInfo {
		using Entity = uint32_t;

		Entity otherEntity = 0xFFFFFFFF;
		Math::vec3 normal;
		float depth = 0.0f;
		std::vector<Math::vec3> relativePoints;
	};

	// Editable Script Variable
	enum class EditableScriptVariableType {
		Boolean,
		Int8,
		Int16,
		Int32,
		Int64,
		Uint8,
		Uint16,
		Uint32,
		Uint64,
		Float32,
		Float64,
		String,
		Vector2,
		Vector3,
		Vector4,
		Quaternion,
		Unknown
	};

	struct EditableScriptVariable {
		EditableScriptVariableType type = EditableScriptVariableType::Unknown;
		void* address;
	};

	// Script
	struct ScriptBase {
		virtual void onCollisionEnter(CollisionInfo collisionInfo) {
			NTSHENGN_UNUSED(collisionInfo);
		}
		virtual void onCollisionStill(CollisionInfo collisionInfo) {
			NTSHENGN_UNUSED(collisionInfo);
		}
		virtual void onCollisionExit(CollisionInfo collisionInfo) {
			NTSHENGN_UNUSED(collisionInfo);
		}

		virtual void createEditableScriptVariableMap() {};

	public:
		std::unordered_map<std::string, EditableScriptVariable> editableScriptVariables;
	};

}