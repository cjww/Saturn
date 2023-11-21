#pragma once
#include "Tools\Matrix.h"
#include <Window.hpp>
#include "Tools/Vector.h"

namespace sa {

	struct Rectf {
		glm::vec2 offset;
		glm::vec2 extent;
	};

	enum ProjectionMode {
		ePerspective,
		eOrthographic
	};

	class SceneCamera {
	private:
		Rectf m_viewport;
		Matrix4x4 m_projMat;

		Vector3 m_position;
		Vector3 m_forward;
		Vector3 m_up;

		ProjectionMode m_projectionMode;

		float m_fov;
		float m_apectRatio;
		float m_near = 0.01f;
		float m_far = 1000.0f;

		Bounds m_orthoBounds;

		void updateProjection();

	public:
		SceneCamera();
		
		SceneCamera(const SceneCamera&) = default;
		SceneCamera& operator=(const SceneCamera&) = default;
		
		float getFOVRadians() const;
		void setFOVRadians(float fovRadians);
		float getFOVDegrees() const;
		void setFOVDegrees(float fovDegrees);


		Rectf getViewport() const;
		void setViewport(Rectf viewport);
		
		void setAspectRatio(float aspectRatio);

		void lookAt(Vector3 target);
		void lookTo(Vector3 forward);

	
		void setPosition(Vector3 position);
		Vector3 getPosition() const;

		void rotate(float angle, Vector3 axis);

		void setForward(Vector3 forward);
		Vector3 getForward() const;
		Vector3 getUp() const;
		Vector3 getRight() const;

		Matrix4x4 getViewMatrix() const;
		Matrix4x4 getProjectionMatrix() const;


		float getNear() const;
		void setNear(float value);
		float getFar() const;
		void setFar(float value);

		Bounds getOrthoBounds() const;
		void setOrthoBounds(Bounds bounds);
		
		float getOrthoWidth() const;
		void setOrthoWidth(float width);


		ProjectionMode getProjectionMode() const;
		void setProjectionMode(ProjectionMode projectionMode);

		
	};
}