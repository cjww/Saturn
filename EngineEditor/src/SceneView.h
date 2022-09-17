#pragma once
#include <Camera.h>
#include "EditorModule.h"
#include <Tools\Math.h>
#include "ECS/Components.h"

#include <glm\gtc\matrix_transform.hpp>

class SceneView : public EditorModule {
private:
	sa::Camera m_camera;
	sa::RenderWindow* m_pWindow;

	bool m_isFocused;
	sa::Vector2 m_lastMousePos;
	float m_mouseSensitivity;
	float m_moveSpeed;

	ImVec2 m_displayedSize;
	float m_deltaTime;
	bool m_isDraging;
	sa::Vector3 m_dragDirection;

	sa::Entity m_selectedEntity;
	bool m_isWorldCoordinates;

	struct Statistics {
		float frameTime;
		sa::DeviceMemoryStats gpuMemoryStats;
	} m_statistics;
	float m_statsUpdateTime;
	float m_statsTimer;
	std::array<float, 100> m_frameTimeGraph;



public:
	SceneView(sa::Engine* pEngine, sa::RenderWindow* pWindow);
	virtual ~SceneView() override;

	// moves camera around scene according to input
	void update(float dt);

	virtual void onImGui() override;

	void imGuiDrawLine(glm::vec3 p1, glm::vec3 p2, const ImColor& color, float thickness = 1.0f);
	bool imGuiDrawVector(glm::vec3 origin, glm::vec3 v, const ImColor& color, float thickness = 1.0f);
	bool imGuiDrawVector(glm::vec3 v, const ImColor& color, float thickness = 1.0f);


	sa::Camera* getCamera();

	sa::Entity getEntity() const;
	void setEntity(sa::Entity entity);


};