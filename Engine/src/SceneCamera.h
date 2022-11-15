#pragma once
#include "Tools\Matrix.h"
#include <Window.hpp>
#include "Tools/Vector.h"

namespace sa {
	class SceneCamera {
	private:
		Rect m_viewport;
		Matrix4x4 m_projMat;

		Vector3 m_position;
		Vector3 m_forward;
		Vector3 m_up;


		float m_fov;
		float m_apectRatio;
		inline static const float m_near = 0.01f;
		inline static const float m_far = 1000.0f;

		void updateProjection();

	public:
		SceneCamera();
		SceneCamera(const Window* pWindow);
		SceneCamera(Extent windowExtent);
		
		SceneCamera(const SceneCamera&) = default;
		SceneCamera& operator=(const SceneCamera&) = default;
		
		void setFOVRadians(float fovRadians);
		void setFOVDegrees(float fovDegrees);

		void setViewport(Rect viewport);
		
		void setAspectRatio(float aspectRatio);

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

		float getNear() const;
		float getFar() const;


	};
}