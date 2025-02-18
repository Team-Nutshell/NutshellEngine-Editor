#pragma once
#include "../Common/script/ntshengn_script.h"

using namespace NtshEngn;
struct GameControllerScript : public Script {
	NTSHENGN_SCRIPT(GameControllerScript);

	void init() {

	}

	void update(float dt) {
		NTSHENGN_UNUSED(dt);

		if (getConnectedGamepads().size() > 0) {
			gamepad = getConnectedGamepads()[0];
		} else {
			gamepad = NTSHENGN_GAMEPAD_UNKNOWN;
			keyboardMode = true;
		}

		if ((getKeyState(InputKeyboardKey::Any) == InputState::Pressed) ||
			(getMouseButtonState(InputMouseButton::Any) == InputState::Pressed)) {
			keyboardMode = true;
		}
		else if ((gamepad != NTSHENGN_GAMEPAD_UNKNOWN) &&
			((getGamepadButtonState(gamepad, InputGamepadButton::Any) == InputState::Pressed) ||
			(std::abs(getGamepadStickAxisX(gamepad, InputGamepadStick::Left)) > 0.25f) ||
			(std::abs(getGamepadStickAxisX(gamepad, InputGamepadStick::Right)) > 0.25f) ||
			(std::abs(getGamepadStickAxisY(gamepad, InputGamepadStick::Left)) > 0.25f) ||
			(std::abs(getGamepadStickAxisY(gamepad, InputGamepadStick::Right)) > 0.25f))) {
			keyboardMode = false;
		}

		if ((getKeyState(InputKeyboardKey::Alt) == InputState::Held) && (getKeyState(InputKeyboardKey::Enter) == InputState::Pressed)) {
			setWindowFullscreen(!isWindowFullscreen());
		}
	}

	void destroy() {

	}

public:
	GamepadID gamepad = NTSHENGN_GAMEPAD_UNKNOWN;

	bool keyboardMode = true;
};