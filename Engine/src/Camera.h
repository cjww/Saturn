#pragma once
#include "glm\gtc\matrix_transform.hpp"
#include "glm\gtx\rotate_vector.hpp"

#include "Geometry\Rect.h"
#include <RenderWindow.hpp>

namespace sa {
	class Camera {
	private:
		Rect m_viewport;
		glm::mat4 m_projMat;

		glm::vec3 m_position;
		glm::vec3 m_forward;
		glm::vec3 m_up;


		float m_fov;
		float m_apectRatio;
		const float m_near = 0.001f;
		const float m_far = 1000.0f;
	public:
		Camera();
		Camera(const RenderWindow* pWindow);

		void setFOVRadians(float fovRadians);
		void setFOVDegrees(float fovDegrees);

		void setViewport(Rect viewport);

		void lookAt(glm::vec3 target);
	
		void setPosition(glm::vec3 position);
		glm::vec3 getPosition() const;

		void rotate(float angle, glm::vec3 axis);

		glm::vec3 getForward() const;
		glm::vec3 getUp() const;
		glm::vec3 getRight() const;

		glm::mat4 getViewMatrix();
		glm::mat4 getProjectionMatrix();

		Rect getViewport() const;

	};
}