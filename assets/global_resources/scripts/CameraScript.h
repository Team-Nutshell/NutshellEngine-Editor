#pragma once
#include "../Common/script/ntshengn_script.h"

using namespace NtshEngn;
struct CameraScript : public Script {
	NTSHENGN_SCRIPT(CameraScript);

	void init() {
		setCursorVisibility(!m_mouseMiddleMode);

		m_prevMouseX = getWindowWidth() / 2;
		m_prevMouseY = getWindowHeight() / 2;
		setCursorPosition(m_prevMouseX, m_prevMouseY);

		const Transform& transform = getEntityComponent<Transform>(entityID);

		m_yaw = Math::toDeg(transform.rotation.y);
		m_pitch = Math::toDeg(transform.rotation.x);

		const Camera& camera = getEntityComponent<Camera>(entityID);
		Math::vec3 cameraForward = Math::normalize(camera.forward);

		m_forwardYaw = std::atan2(cameraForward.z, cameraForward.x);
		m_forwardPitch = -std::asin(cameraForward.y);
	}

	void update(float dt) {
		GamepadID gamepad = getConnectedGamepads().empty() ? NTSHENGN_GAMEPAD_UNKNOWN : getConnectedGamepads()[0];
		if (gamepad != NTSHENGN_GAMEPAD_UNKNOWN) {
			if ((getGamepadButtonState(gamepad, InputGamepadButton::Any) == InputState::Pressed) ||
				(getGamepadLeftTrigger(gamepad) > 0.1f) ||
				(getGamepadRightTrigger(gamepad) > 0.1f) ||
				(std::abs(getGamepadStickAxisX(gamepad, InputGamepadStick::Left)) > 0.1f) ||
				(std::abs(getGamepadStickAxisY(gamepad, InputGamepadStick::Left)) > 0.1f) ||
				(std::abs(getGamepadStickAxisX(gamepad, InputGamepadStick::Right)) > 0.1f) ||
				(std::abs(getGamepadStickAxisY(gamepad, InputGamepadStick::Right)) > 0.1f)) {
				m_keyboardMode = false;
			}
		}
		else {
			m_keyboardMode = true;
		}

		if (getKeyState(InputKeyboardKey::Any) == InputState::Pressed) {
			m_keyboardMode = true;
		}

		if (m_keyboardMode) {
			if (getKeyState(InputKeyboardKey::Escape) == InputState::Pressed) {
				m_mouseMiddleMode = !m_mouseMiddleMode;
				setCursorVisibility(!m_mouseMiddleMode);
				if (m_mouseMiddleMode) {
					m_prevMouseX = getWindowWidth() / 2;
					m_prevMouseY = getWindowHeight() / 2;
					setCursorPosition(m_prevMouseX, m_prevMouseY);
				}
			}
		}
		else {
			if (getGamepadButtonState(gamepad, InputGamepadButton::Start) == InputState::Pressed) {
				m_mouseMiddleMode = !m_mouseMiddleMode;
				setCursorVisibility(!m_mouseMiddleMode);
				if (m_mouseMiddleMode) {
					m_prevMouseX = getWindowWidth() / 2;
					m_prevMouseY = getWindowHeight() / 2;
					setCursorPosition(m_prevMouseX, m_prevMouseY);
				}
			}
		}

		Transform& transform = getEntityComponent<Transform>(entityID);

		if (m_keyboardMode) {
			if (m_mouseMiddleMode) {
				const int mouseX = getCursorPositionX();
				const int mouseY = getCursorPositionY();

				m_prevMouseX = getWindowWidth() / 2;
				m_prevMouseY = getWindowHeight() / 2;
				setCursorPosition(m_prevMouseX, m_prevMouseY);

				const float xOffset = (mouseX - m_prevMouseX) * m_mouseSensitivity;
				const float yOffset = (mouseY - m_prevMouseY) * m_mouseSensitivity;

				m_prevMouseX = mouseX;
				m_prevMouseY = mouseY;

				m_yaw = std::fmod(m_yaw + xOffset, 360.0f);
				m_pitch = std::max(-89.0f, std::min(89.0f, m_pitch + yOffset));
			}
		}
		else {
			Math::vec2 gamepadDirection;
			gamepadDirection.x = getGamepadStickAxisX(gamepad, InputGamepadStick::Right);
			if (std::abs(gamepadDirection.x) < 0.1f) {
				gamepadDirection.x = 0.0f;
			}
			gamepadDirection.y = getGamepadStickAxisY(gamepad, InputGamepadStick::Right);
			if (std::abs(gamepadDirection.y) < 0.1f) {
				gamepadDirection.y = 0.0f;
			}

			const float xOffset = gamepadDirection.x * m_mouseSensitivity * 10.0f;
			const float yOffset = gamepadDirection.y * m_mouseSensitivity * 10.0f;

			m_yaw = std::fmod(m_yaw + xOffset, 360.0f);
			m_pitch = std::max(-89.0f, std::min(89.0f, m_pitch + yOffset));
		}

		float yawRad = Math::toRad(m_yaw);
		float pitchRad = Math::toRad(m_pitch);

		transform.rotation.x = pitchRad;
		transform.rotation.y = yawRad;

		Math::vec3 newForward;
		newForward.x = std::cos(m_forwardPitch + pitchRad) * std::cos(m_forwardYaw + yawRad);
		newForward.y = -std::sin(m_forwardPitch + pitchRad);
		newForward.z = std::cos(m_forwardPitch + pitchRad) * std::sin(m_forwardYaw + yawRad);
		newForward = Math::normalize(newForward);

		Math::vec3 addedPosition = Math::vec3(0.0f, 0.0f, 0.0f);
		const float cameraSpeed = m_cameraSpeed * dt;

		if (m_keyboardMode) {
			if ((getKeyState(InputKeyboardKey::W) == InputState::Held)) {
				addedPosition += (newForward * cameraSpeed);
			}
			if (getKeyState(InputKeyboardKey::S) == InputState::Held) {
				addedPosition -= (newForward * cameraSpeed);
			}
			if (getKeyState(InputKeyboardKey::A) == InputState::Held) {
				Math::vec3 t = Math::normalize(Math::vec3(-newForward.z, 0.0, newForward.x));
				addedPosition.x -= (t.x * cameraSpeed);
				addedPosition.z -= (t.z * cameraSpeed);
			}
			if (getKeyState(InputKeyboardKey::D) == InputState::Held) {
				Math::vec3 t = Math::normalize(Math::vec3(-newForward.z, 0.0, newForward.x));
				addedPosition.x += (t.x * cameraSpeed);
				addedPosition.z += (t.z * cameraSpeed);
			}
			if (getKeyState(InputKeyboardKey::Space) == InputState::Held) {
				addedPosition.y += cameraSpeed;
			}
			if (getKeyState(InputKeyboardKey::Shift) == InputState::Held) {
				addedPosition.y -= cameraSpeed;
			}
		}
		else {
			Math::vec3 t = Math::normalize(Math::vec3(-newForward.z, 0.0, newForward.x));
			Math::vec2 gamepadDirection = Math::vec2(0.0f, 0.0f);
			gamepadDirection.x = getGamepadStickAxisX(gamepad, InputGamepadStick::Left);
			if (std::abs(gamepadDirection.x) < 0.1f) {
				gamepadDirection.x = 0.0f;
			}
			gamepadDirection.y = getGamepadStickAxisY(gamepad, InputGamepadStick::Left);
			if (std::abs(gamepadDirection.y) < 0.1f) {
				gamepadDirection.y = 0.0f;
			}
			addedPosition.x += (t.x * gamepadDirection.x * cameraSpeed);
			addedPosition.z += (t.z * gamepadDirection.x * cameraSpeed);

			float leftTrigger = getGamepadLeftTrigger(gamepad);
			if (leftTrigger > 0.1f) {
				addedPosition.y -= leftTrigger * cameraSpeed;
			}
			float rightTrigger = getGamepadRightTrigger(gamepad);
			if (rightTrigger > 0.1f) {
				addedPosition.y += rightTrigger * cameraSpeed;
			}

			addedPosition += (newForward * -gamepadDirection.y * cameraSpeed); 
		}

		if (Math::dot(addedPosition, addedPosition) > (cameraSpeed * cameraSpeed)) {
			addedPosition = Math::normalize(addedPosition) * cameraSpeed;
		}
		transform.position += addedPosition;
	}

	void destroy() {
	}

private:
	bool m_keyboardMode = true;

	bool m_mouseMiddleMode = false;

	NTSHENGN_EDITABLE_VARIABLE float m_cameraSpeed = 1.5f;
	NTSHENGN_EDITABLE_VARIABLE float m_mouseSensitivity = 0.12f;

	int m_prevMouseX = 0;
	int m_prevMouseY = 0;

	float m_forwardYaw = 0.0f;
	float m_forwardPitch = 0.0f;

	float m_yaw = 0.0f;
	float m_pitch = 0.0f;
};