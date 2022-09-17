#include "SceneView.h"

#include "Graphics\RenderTechniques\ForwardPlus.h"

SceneView::SceneView(sa::Engine* pEngine, sa::RenderWindow* pWindow)
	: EditorModule(pEngine)
{
	m_pWindow = pWindow;
	m_isFocused = false;
	m_isWorldCoordinates = false;
	m_selectedEntity = {};

	sa::Rect viewport = { { 0, 0 }, pWindow->getCurrentExtent() };
	m_camera.setViewport(viewport);

	m_camera.setPosition(sa::Vector3(0, 0, 5));
	m_camera.lookAt(sa::Vector3(0, 0, 0));

	m_mouseSensitivity = 30.0f;
	m_moveSpeed = 8.0f;

	m_statsUpdateTime = 0.1f;
	m_statsTimer = m_statsUpdateTime;

	pEngine->on<sa::engine_event::SceneSet>([&](const sa::engine_event::SceneSet& sceneSetEvent, sa::Engine& engine) {
		sceneSetEvent.newScene->on<sa::editor_event::EntitySelected>([&](const sa::editor_event::EntitySelected& e, sa::Scene&) {
			m_selectedEntity = e.entity;
		});

		sceneSetEvent.newScene->on<sa::editor_event::EntityDeselected>([&](const sa::editor_event::EntityDeselected&, sa::Scene&) {
			m_selectedEntity = {};
		});
	});
}

SceneView::~SceneView() {

}

void SceneView::update(float dt) {
	SA_PROFILE_FUNCTION();

	m_deltaTime = dt;
	
	m_statsTimer -= dt;
	if (m_statsTimer < 0.0f) {
		m_statsTimer = m_statsUpdateTime;
		m_statistics.frameTime = dt;
		m_statistics.gpuMemoryStats = sa::Renderer::get().getGPUMemoryUsage();
		
		std::copy(m_frameTimeGraph.begin() + 1, m_frameTimeGraph.end(), m_frameTimeGraph.begin());
		m_frameTimeGraph[m_frameTimeGraph.size() - 1] = dt * 1000;
		
	}

	if (!m_isFocused) {
		m_lastMousePos = m_pWindow->getCursorPosition();
		return;
	}

	sa::Vector2 mousePos = m_pWindow->getCursorPosition();
	if (m_pWindow->getMouseButton(sa::MouseButton::BUTTON_2)) {
		sa::Vector2 delta = m_lastMousePos - mousePos;
		m_camera.rotate(glm::radians(delta.x) * dt * m_mouseSensitivity, { 0, 1, 0 });
		m_camera.rotate(glm::radians(delta.y) * dt * m_mouseSensitivity, m_camera.getRight());
	}
	m_lastMousePos = mousePos;
		
	int forward = m_pWindow->getKey(sa::Key::W) - m_pWindow->getKey(sa::Key::S);
	int right = m_pWindow->getKey(sa::Key::D) - m_pWindow->getKey(sa::Key::A);
	int up = m_pWindow->getKey(sa::Key::Q) - m_pWindow->getKey(sa::Key::E);

	glm::vec3 camPos = m_camera.getPosition();
	camPos += (float)forward * dt * m_moveSpeed * m_camera.getForward();
	camPos += (float)right * dt * m_moveSpeed * m_camera.getRight();
	camPos += (float)up * dt * m_moveSpeed * m_camera.getUp();

	m_camera.setPosition(camPos);

}

void SceneView::onImGui() {
	SA_PROFILE_FUNCTION();

	if (ImGui::Begin("Scene view", 0, ImGuiWindowFlags_MenuBar)) {
		if (ImGui::BeginMenuBar()) {

			if (ImGui::BeginMenu("Menu")) {


				for (int i = 0; i < 5; i++) {
					ImGui::Button("something");
				}
				ImGui::EndMenu();
			}

			static bool showStats = false;
			//ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() - ImGui::GetFontSize() * 8);
			ImGui::Checkbox("Statistics", &showStats);
			if (showStats) {
				ImGui::SetNextWindowBgAlpha(0.3f);
				if (ImGui::Begin("Statistics", 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize)) {

					ImGui::Text("Entity Count: %llu", m_pEngine->getCurrentScene()->getEntityCount());
					
					ImGui::Text("FPS: %f", 1 / m_statistics.frameTime);
					ImGui::Text("Frame time: %f ms", m_statistics.frameTime * 1000);
					ImGui::PlotLines("Frame time", m_frameTimeGraph.data(), m_frameTimeGraph.size(), 0, 0, 0.0f, 100.f, ImVec2{0, 50});
				
					ImGui::Text("GPU Memory usage");
					size_t totalUsage = 0;
					size_t totalBudget = 0;

					ImGui::Indent();
					for (auto& heap : m_statistics.gpuMemoryStats.heaps) {
						ImGui::Text("%llu MB / %llu MB, flags: %u", heap.usage / 1000000, heap.budget / 1000000, heap.flags);
						totalUsage += heap.usage;
						totalBudget += heap.budget;
					}
					ImGui::Text("Total usage: %llu MB / %llu MB", totalUsage / 1000000, totalBudget / 1000000);
					ImGui::Unindent();
				}
				ImGui::End();
			}
			
			
			
			static bool showLightHeatmap = false;
			sa::ForwardPlus* forwardPlusTechnique = dynamic_cast<sa::ForwardPlus*>(m_pEngine->getRenderPipeline().getRenderTechnique());
			if (forwardPlusTechnique) {
				if (ImGui::Checkbox("Show Light Heatmap", &showLightHeatmap)) {
					forwardPlusTechnique->setShowHeatmap(showLightHeatmap);
				}
			}
			
			ImGui::Checkbox("World Coordinates", &m_isWorldCoordinates);

		}
		ImGui::EndMenuBar();

		sa::Scene* pScene = m_pEngine->getCurrentScene();
	

		m_isFocused = ImGui::IsWindowFocused();

		// render outputTexture with constant aspect ratio
		ImVec2 availSize = ImGui::GetContentRegionAvail();

		sa::Texture texture = m_pEngine->getRenderPipeline().getRenderTechnique()->getOutputTexture();
		float aspectRatio = (float)texture.getExtent().height / texture.getExtent().width;
		availSize.y = std::min(availSize.x * aspectRatio, (float)texture.getExtent().height);
		ImGui::Image(texture, availSize);
		m_displayedSize = availSize;

		const ImU32 red = ImColor(ImVec4(1, 0, 0, 1));
		const ImU32 green = ImColor(ImVec4(0, 1, 0, 1));
		const ImU32 blue = ImColor(ImVec4(0, 0, 1, 1));
		
		if (m_selectedEntity) {
			comp::Transform* transform = m_selectedEntity.getComponent<comp::Transform>();

			if (transform) {
				glm::vec3 pos = transform->position;

				glm::vec3 x = { 1, 0, 0 };
				glm::vec3 y = { 0, 1, 0 };
				glm::vec3 z = { 0, 0, 1 };

				if (!m_isWorldCoordinates) {
					glm::mat4 mat = glm::toMat4(transform->rotation);
					x = glm::normalize(mat[0]);
					y = glm::normalize(mat[1]);
					z = glm::normalize(mat[2]);
				}

				bool mouseDown = ImGui::IsMouseDown(ImGuiMouseButton_Left);
				if (m_isDraging && !mouseDown) {
					m_isDraging = false;
				}

				if (m_isDraging) {

					ImVec2 imWindowPos = ImGui::GetWindowPos();
					ImVec2 contentMin = ImGui::GetWindowContentRegionMin();
				
					glm::vec2 windowPos = { imWindowPos.x, imWindowPos.y };
					glm::vec2 min = { contentMin.x, contentMin.y };
					windowPos += min;
				
					glm::vec2 screenSize = { m_displayedSize.x, m_displayedSize.y };
				
					
					ImVec2 imDragDelta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left, 0.0f);
					ImGui::ResetMouseDragDelta();

					ImVec2 imMousePos = ImGui::GetMousePos();
					glm::vec2 mousePos = { imMousePos.x, imMousePos.y };
					glm::vec2 lastMousePos = { imMousePos.x - imDragDelta.x, imMousePos.y - imDragDelta.y };
					
					glm::vec3 mousePosWorld = sa::math::screenToWorld(mousePos, &m_camera, windowPos, screenSize);
					mousePosWorld = m_camera.getPosition() + mousePosWorld * glm::length(pos - m_camera.getPosition());

					glm::vec3 lastMousePosWorld = sa::math::screenToWorld(lastMousePos, &m_camera, windowPos, screenSize);
					lastMousePosWorld = m_camera.getPosition() + lastMousePosWorld * glm::length(pos - m_camera.getPosition());

					glm::vec3 dragDeltaWorld = mousePosWorld - lastMousePosWorld;
					
					float angle = glm::dot(dragDeltaWorld, m_dragDirection);

					transform->position += m_dragDirection * angle * 90.f;
					
				}
				
				if (imGuiDrawVector(pos, x, red, 1)) {
					if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
						m_isDraging = true;
						m_dragDirection = x;
					}
				}
				
				if(imGuiDrawVector(pos, y, green, 1)) {
					if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
						m_isDraging = true;
						m_dragDirection = y;
					}
				}

				if (imGuiDrawVector(pos, z, blue, 1)) {
					if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
						m_isDraging = true;
						m_dragDirection = z;
					}
				}

			}
		}
		

	}
	ImGui::End();


}

void SceneView::imGuiDrawLine(glm::vec3 p1, glm::vec3 p2, const ImColor& color, float thickness) {
	ImVec2 contentMin = ImGui::GetWindowContentRegionMin();
	ImVec2 windowPos = ImGui::GetWindowPos();
	contentMin.x += windowPos.x;
	contentMin.y += windowPos.y;

	glm::vec3 forward = m_camera.getForward();

	glm::vec3 cameraToPos1 = p1 - m_camera.getPosition();
	glm::vec3 cameraToPos2 = p2 - m_camera.getPosition();

	ImVec2 availSize = m_displayedSize;
	if (glm::dot(cameraToPos1, forward) > 0 && glm::dot(cameraToPos2, forward) > 0) {

		p1 = sa::math::worldToScreen(p1, &m_camera, 
			{ contentMin.x, contentMin.y },
			{ m_displayedSize.x, m_displayedSize.y });
		p2 = sa::math::worldToScreen(p2, &m_camera, 
			{ contentMin.x, contentMin.y }, 
			{ m_displayedSize.x, m_displayedSize.y });

		ImGui::GetWindowDrawList()->AddLine({ p1.x, p1.y }, { p2.x, p2.y }, color, thickness);

	}
}

bool SceneView::imGuiDrawVector(glm::vec3 origin, glm::vec3 v, const ImColor& color, float thickness) {
	ImVec2 contentMin = ImGui::GetWindowContentRegionMin();
	ImVec2 windowPos = ImGui::GetWindowPos();
	contentMin.x += windowPos.x;
	contentMin.y += windowPos.y;


	glm::vec3 cameraToPos = v - m_camera.getPosition();

	glm::vec3 forward = m_camera.getForward();

	if (glm::dot(cameraToPos, forward) > 0) {
		glm::vec3 o = sa::math::worldToScreen(origin, &m_camera,
			{ contentMin.x, contentMin.y },
			{ m_displayedSize.x, m_displayedSize.y });

		glm::vec3 end = sa::math::worldToScreen(origin + v, &m_camera,
			{ contentMin.x, contentMin.y },
			{ m_displayedSize.x, m_displayedSize.y });

		ImGui::GetWindowDrawList()->AddLine({ o.x, o.y }, { end.x, end.y }, color, thickness);

		glm::vec3 dir = v;
		dir = glm::normalize(dir);
		glm::vec3 normal = forward;
		normal = glm::normalize(normal);
		glm::vec3 tangent = glm::cross(dir, normal);
		tangent = glm::normalize(tangent);

		float size = 0.02f * thickness;
		glm::vec3 p2 = origin + v;
		p2 -= dir * size * 4.f;
		glm::vec3 p3 = p2;

		p2 += tangent * size;
		p3 -= tangent * size;
		
		p2 = sa::math::worldToScreen(p2, &m_camera,
			{ contentMin.x, contentMin.y },
			{ m_displayedSize.x, m_displayedSize.y });
		p3 = sa::math::worldToScreen(p3, &m_camera,
			{ contentMin.x, contentMin.y },
			{ m_displayedSize.x, m_displayedSize.y });

		ImGui::GetWindowDrawList()->AddTriangleFilled({ end.x, end.y }, { p2.x, p2.y }, { p3.x, p3.y }, color);

		glm::vec3 min = glm::min(glm::min(glm::min(end, p2), p3), o);
		glm::vec3 max = glm::max(glm::max(glm::max(end, p2), p3), o);

		ImVec2 arrowRectMin = { min.x, min.y };
		ImVec2 arrowRectMax = { max.x, max.y };

		if (ImGui::IsMouseHoveringRect(arrowRectMin, arrowRectMax)) {
			return true;
		}
	}

	return false;
}

bool SceneView::imGuiDrawVector(glm::vec3 v, const ImColor& color, float thickness) {
	return imGuiDrawVector({ 0, 0, 0 }, v, color, thickness);
}


sa::Camera* SceneView::getCamera() {
	return &m_camera;
}

sa::Entity SceneView::getEntity() const {
	return m_selectedEntity;
}

void SceneView::setEntity(sa::Entity entity) {
	m_selectedEntity = entity;
}
