#pragma once
#include <Camera.h>
#include "EditorModule.h"
#include <Tools\Math.h>
#include "ECS/Components.h"

#include <glm\gtc\matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <ImGuizmo.h>

#include "CustomImGui.h"

class SceneView : public EditorModule {
private:
	sa::Camera m_camera;
	sa::RenderWindow* m_pWindow;

	bool m_isFocused;
	sa::Vector2 m_lastMousePos;
	float m_mouseSensitivity;

	glm::vec3 m_velocity;
	float m_maxVelocityMagnitude;
	float m_acceleration;

	glm::vec2 m_displayedSize;
	float m_deltaTime;
	
	sa::Entity m_selectedEntity;
	bool m_isWorldCoordinates;
	
	float m_zoom;

	struct Statistics {
		float frameTime;
		sa::DeviceMemoryStats gpuMemoryStats;
		size_t totalGPUMemoryUsage;
		size_t totalGPUMemoryBudget;

	} m_statistics;
	float m_statsUpdateTime;
	float m_statsTimer;
	std::array<float, 100> m_frameTimeGraph;
	std::array<float, 100> m_gpuMemoryData;




public:
	SceneView(sa::Engine* pEngine, sa::EngineEditor* pEditor, sa::RenderWindow* pWindow);
	virtual ~SceneView() override;

	// moves camera around scene according to input
	virtual void update(float dt) override;

	virtual void onImGui() override;

	void imGuiDrawLine(glm::vec3 p1, glm::vec3 p2, const ImColor& color, float thickness = 1.0f);
	bool imGuiDrawVector(glm::vec3 origin, glm::vec3 v, const ImColor& color, float thickness = 1.0f);
	bool imGuiDrawVector(glm::vec3 v, const ImColor& color, float thickness = 1.0f);


	sa::Camera* getCamera();

	sa::Entity getEntity() const;
	void setEntity(sa::Entity entity);


};