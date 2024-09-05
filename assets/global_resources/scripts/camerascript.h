#pragma once
#include "../Common/script/ntshengn_script.h"
#include <cmath>

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

	void update(double dt) {
		if (getKeyState(InputKeyboardKey::Escape) == InputState::Pressed) {
			m_mouseMiddleMode = !m_mouseMiddleMode;
			setCursorVisibility(!m_mouseMiddleMode);
			if (m_mouseMiddleMode) {
				m_prevMouseX = getWindowWidth() / 2;
				m_prevMouseY = getWindowHeight() / 2;
				setCursorPosition(m_prevMouseX, m_prevMouseY);
			}
		}

		Transform& transform = getEntityComponent<Transform>(entityID);

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

		float yawRad = Math::toRad(m_yaw);
		float pitchRad = Math::toRad(m_pitch);

		transform.rotation.x = pitchRad;
		transform.rotation.y = yawRad;

		Math::vec3 newForward;
		newForward.x = std::cos(m_forwardPitch + pitchRad) * std::cos(m_forwardYaw + yawRad);
		newForward.y = -std::sin(m_forwardPitch + pitchRad);
		newForward.z = std::cos(m_forwardPitch + pitchRad) * std::sin(m_forwardYaw + yawRad);
		newForward = Math::normalize(newForward);

		const float cameraSpeed = m_cameraSpeed * static_cast<float>(dt);

		if (getKeyState(InputKeyboardKey::W) == InputState::Held) {
			transform.position += (newForward * cameraSpeed);
		}
		if (getKeyState(InputKeyboardKey::S) == InputState::Held) {
			transform.position -= (newForward * cameraSpeed);
		}
		if (getKeyState(InputKeyboardKey::A) == InputState::Held) {
			Math::vec3 t = Math::normalize(Math::vec3(-newForward.z, 0.0, newForward.x));
			transform.position.x -= (t.x * cameraSpeed);
			transform.position.z -= (t.z * cameraSpeed);
		}
		if (getKeyState(InputKeyboardKey::D) == InputState::Held) {
			Math::vec3 t = Math::normalize(Math::vec3(-newForward.z, 0.0, newForward.x));
			transform.position.x += (t.x * cameraSpeed);
			transform.position.z += (t.z * cameraSpeed);
		}
		if (getKeyState(InputKeyboardKey::Space) == InputState::Held) {
			transform.position.y += cameraSpeed;
		}
		if (getKeyState(InputKeyboardKey::Shift) == InputState::Held) {
			transform.position.y -= cameraSpeed;
		}
	}

	void destroy() {
	}

private:
	bool m_mouseMiddleMode = false;

	const float m_cameraSpeed = 0.0015f;
	const float m_mouseSensitivity = 0.12f;

	int m_prevMouseX = 0;
	int m_prevMouseY = 0;

	float m_forwardYaw = 0.0f;
	float m_forwardPitch = 0.0f;

	float m_yaw = 0.0f;
	float m_pitch = 0.0f;
};