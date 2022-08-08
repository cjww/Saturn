#pragma once

#include "Camera.h"
#include "Window.hpp"

#include "glm/gtx/rotate_vector.hpp"

namespace sa {

	class CameraController {
	private:
		
		glm::vec2 lastMousePos;

		Camera& m_camera;
		Window& m_window;

		bool escapePressed;

	public:
		float speed = 10.f;
		float sensitivty = 1.f;

		bool mouseLocked;

		CameraController(Window& window, sa::Camera& camera) 
			: m_camera(camera)
			, m_window(window)
		{
			
			mouseLocked = true;
			m_window.setCursorDisabled(true);
		};

		void update(float dt) {
			float hori = m_window.getKey(sa::Key::D) - m_window.getKey(sa::Key::A);
			float vert = m_window.getKey(sa::Key::W) - m_window.getKey(sa::Key::S);

			glm::vec2 mPos = { m_window.getCursorPosition().x, m_window.getCursorPosition().y };
			glm::vec2 center = glm::vec2((float)m_window.getCurrentExtent().width / 2, (float)m_window.getCurrentExtent().height / 2);
			glm::vec2 diff = mPos - center;

			float up = m_window.getKey(sa::Key::SPACE) - m_window.getKey(sa::Key::LEFT_CONTROL);
			bool sprint = m_window.getKey(sa::Key::LEFT_SHIFT);


			if (sa::Window::IsGamepad(sa::Joystick::JOYSTICK_1)) {
				sa::GamepadState state = sa::Window::GetGamepadState(sa::Joystick::JOYSTICK_1);

				hori = state.getAxis(sa::GamepadAxis::LEFT_X);
				vert = state.getAxis(sa::GamepadAxis::LEFT_Y) * -1.f;

				diff.x = state.getAxis(sa::GamepadAxis::RIGHT_X);
				diff.y = state.getAxis(sa::GamepadAxis::RIGHT_Y);

				up = state.getAxis(sa::GamepadAxis::LEFT_TRIGGER) - state.getAxis(sa::GamepadAxis::RIGHT_TRIGGER);

				sprint = state.getButton(sa::GamepadButton::RIGHT_BUMPER);

			}

			if (mouseLocked) {
				m_window.setCursorPosition({ center.x, center.y });
			}
			else {
				diff = glm::vec2(0, 0);
			}

			if (m_window.getKey(sa::Key::ESCAPE) && !escapePressed) {
				mouseLocked = !mouseLocked;
				m_window.setCursorDisabled(mouseLocked);
			}
			escapePressed = m_window.getKey(sa::Key::ESCAPE);

			m_camera.rotate(-diff.x * dt * sensitivty, Vector3(0, 1, 0));
			m_camera.rotate(diff.y * dt * sensitivty, m_camera.getRight());


			float finalSpeed = speed;
			if (sprint) {
				finalSpeed *= 2;
			}

			Vector3 position = m_camera.getPosition();
			position += m_camera.getRight() * hori * dt * finalSpeed;
			position += m_camera.getForward() * vert * dt * finalSpeed;
			position += Vector3(0, 1, 0) * -up * dt * finalSpeed;
			m_camera.setPosition(position);

		}
		
	};
}