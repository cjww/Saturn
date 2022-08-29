#pragma once
#include "Tools\Matrix.h"
#include <Window.hpp>
#include "Tools/Vector.h"

namespace sa {
	class Camera {
	private:
		Rect m_viewport;
		Matrix4x4 m_projMat;

		Vector3 m_position;
		Vector3 m_forward;
		Vector3 m_up;


		float m_fov;
		float m_apectRatio;
		const float m_near = 0.001f;
		const float m_far = 1000.0f;

		void updateProjection();

	public:
		Camera();
		Camera(const Window* pWindow);
		Camera(Extent windowExtent);
		
		Camera(const Camera&) = default;
		Camera& operator=(const Camera&) = default;
		
		void setFOVRadians(float fovRadians);
		void setFOVDegrees(float fovDegrees);

		void setViewport(Rect viewport);

		void lookAt(Vector3 target);
	
		void setPosition(Vector3 position);
		Vector3 getPosition() const;

		void rotate(float angle, Vector3 axis);

		Vector3 getForward() const;
		Vector3 getUp() const;
		Vector3 getRight() const;

		Matrix4x4 getViewMatrix() const;
		Matrix4x4 getProjectionMatrix() const;

		Rect getViewport() const;

	};
}