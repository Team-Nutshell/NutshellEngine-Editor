#pragma once
#include "../Common/script/ntshengn_script.h"
#include "GameControllerScript.h"

using namespace NtshEngn;
struct CameraScript : public Script {
	NTSHENGN_SCRIPT(CameraScript);

	void init() {
		setCursorVisibility(!m_mouseMiddleMode);

		m_prevMouseX = getWindowWidth() / 2;
		m_prevMouseY = getWindowHeight() / 2;
		setCursorPosition(m_prevMouseX, m_prevMouseY);

		const Transform& transform = getEntityComponent<Transform>(entityID);

		const Camera& camera = getEntityComponent<Camera>(entityID);
		Math::vec3 cameraForward = Math::normalize(camera.forward);

		m_yaw = Math::toDeg(transform.rotation.y) + Math::toDeg(std::atan2(cameraForward.z, cameraForward.x));
		m_pitch = Math::toDeg(transform.rotation.x) + Math::toDeg(-std::asin(cameraForward.y));
	}

	void update(float dt) {
		NTSHENGN_UNUSED(dt);

		Entity gameController = findEntityByName("GameController");
		Scriptable& gameControllerScriptable = getEntityComponent<Scriptable>(gameController);
		GameControllerScript* gameControllerScript = static_cast<GameControllerScript*>(gameControllerScriptable.script);

		GamepadID gamepad = getConnectedGamepads().empty() ? NTSHENGN_GAMEPAD_UNKNOWN : getConnectedGamepads()[0];

		if (gameControllerScript->keyboardMode) {
			if (getKeyState(InputKeyboardKey::Escape) == InputState::Pressed) {
				m_mouseMiddleMode = !m_mouseMiddleMode;

				m_prevMouseX = getWindowWidth() / 2;
				m_prevMouseY = getWindowHeight() / 2;
				setCursorPosition(m_prevMouseX, m_prevMouseY);

				setCursorVisibility(!m_mouseMiddleMode);
			}
		}
		else {
			if (getGamepadButtonState(gameControllerScript->gamepad, InputGamepadButton::Start) == InputState::Pressed) {
				m_mouseMiddleMode = !m_mouseMiddleMode;
			}
		}

		if (gameControllerScript->keyboardMode) {
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

			const float xOffset = gamepadDirection.x * m_mouseSensitivity * 20.0f;
			const float yOffset = gamepadDirection.y * m_mouseSensitivity * 20.0f;

			m_yaw = std::fmod(m_yaw + xOffset, 360.0f);
			m_pitch = std::max(-89.0f, std::min(89.0f, m_pitch + yOffset));
		}

		float yawRad = Math::toRad(m_yaw);
		float pitchRad = Math::toRad(m_pitch);

		Math::vec3 newForward;
		newForward.x = std::cos(pitchRad) * std::cos(yawRad);
		newForward.y = -std::sin(pitchRad);
		newForward.z = std::cos(pitchRad) * std::sin(yawRad);
		newForward = Math::normalize(newForward);

		Camera& camera = getEntityComponent<Camera>(entityID);
		camera.forward = newForward;

		Rigidbody& rigidbody = getEntityComponent<Rigidbody>(entityID);
		rigidbody.linearVelocity.x = 0.0f;
		rigidbody.linearVelocity.z = 0.0f;

		Math::vec3 addedVelocity = Math::vec3(0.0f, 0.0f, 0.0f);

		if (gameControllerScript->keyboardMode) {
			if (getKeyState(InputKeyboardKey::W) == InputState::Held) {
				Math::vec3 t = Math::normalize(Math::vec3(newForward.x, 0.0, newForward.z));
				addedVelocity += (t * m_cameraSpeed);
			}
			if (getKeyState(InputKeyboardKey::S) == InputState::Held) {
				Math::vec3 t = Math::normalize(Math::vec3(newForward.x, 0.0, newForward.z));
				addedVelocity -= (t * m_cameraSpeed);
			}
			if (getKeyState(InputKeyboardKey::A) == InputState::Held) {
				Math::vec3 t = Math::normalize(Math::vec3(-newForward.z, 0.0, newForward.x));
				addedVelocity.x -= (t.x * m_cameraSpeed);
				addedVelocity.z -= (t.z * m_cameraSpeed);
			}
			if (getKeyState(InputKeyboardKey::D) == InputState::Held) {
				Math::vec3 t = Math::normalize(Math::vec3(-newForward.z, 0.0, newForward.x));
				addedVelocity.x += (t.x * m_cameraSpeed);
				addedVelocity.z += (t.z * m_cameraSpeed);
			}

			if (m_onTheGround && (getKeyState(InputKeyboardKey::Space) == InputState::Pressed)) {
				addedVelocity.y += m_jumpSpeed;
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
			addedVelocity.x += (t.x * gamepadDirection.x * m_cameraSpeed);
			addedVelocity.z += (t.z * gamepadDirection.x * m_cameraSpeed);
			addedVelocity.x += (newForward.x * -gamepadDirection.y * m_cameraSpeed);
			addedVelocity.z += (newForward.z * -gamepadDirection.y * m_cameraSpeed);

			if (m_onTheGround && (getGamepadButtonState(gamepad, InputGamepadButton::Face1) == InputState::Pressed)) {
				addedVelocity.y = m_jumpSpeed;
			}
		}

		if (Math::dot(addedVelocity, addedVelocity) > 0.0f) {
			Math::vec3 normalizedVelocity = Math::normalize(addedVelocity);
			addedVelocity.x = normalizedVelocity.x * m_cameraSpeed;
			addedVelocity.z = normalizedVelocity.z * m_cameraSpeed;
		}
		rigidbody.linearVelocity += addedVelocity;

		getEntityComponent<SoundListener>(entityID).forward = camera.forward;

		m_onTheGround = false;
	}

	void onCollisionEnter(CollisionInfo collisionInfo) {
		if (Math::dot(collisionInfo.normal, Math::vec3(0.0f, -1.0f, 0.0f)) > 0.6f) {
			m_onTheGround = true;
		}
	}

	void onCollisionStill(CollisionInfo collisionInfo) {
		if (Math::dot(collisionInfo.normal, Math::vec3(0.0f, -1.0f, 0.0f)) > 0.6f) {
			m_onTheGround = true;
		}
	}

	void destroy() {
	}

private:
	bool m_mouseMiddleMode = true;

	NTSHENGN_EDITABLE_VARIABLE float m_cameraSpeed = 0.0f;
	NTSHENGN_EDITABLE_VARIABLE float m_jumpSpeed = 0.0f;
	const float m_mouseSensitivity = 0.12f;

	float m_yaw = 0.0f;
	float m_pitch = 0.0f;

	int m_prevMouseX = 0;
	int m_prevMouseY = 0;

	bool m_onTheGround = false;
};