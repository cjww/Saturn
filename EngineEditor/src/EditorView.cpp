#include "EditorView.h"

EditorView::EditorView(sa::Engine* pEngine, sa::RenderWindow* pWindow)
	: EditorModule(pEngine)
{
	m_pWindow = pWindow;
	m_isFocused = false;
	m_selectedEntity = {};

	sa::Rect viewport = { { 0, 0 }, pWindow->getCurrentExtent() };
	m_camera.setViewport(viewport);

	m_camera.setPosition(sa::Vector3(0, 0, 1));
	m_camera.lookAt(sa::Vector3(0, 0, 0));

	m_mouseSensitivity = 30.0f;
	m_moveSpeed = 8.0f;

	m_texture = m_pEngine->getRenderTechnique()->getOutputTexture();
	m_sampler = sa::Renderer::get().createSampler(sa::FilterMode::NEAREST);

	m_statsUpdateTime = 0.5f;
	m_statsTimer = m_statsUpdateTime;
}

EditorView::~EditorView() {

}

void EditorView::update(float dt) {
	SA_PROFILE_FUNCTION();

	m_statsTimer -= dt;
	if (m_statsTimer < 0.0f) {
		m_statsTimer = m_statsUpdateTime;
		
		m_statistics.frameTime = dt;
		m_statistics.gpuMemoryStats = sa::Renderer::get().getGPUMemoryUsage();
	}

	if (!m_isFocused) {
		m_lastMousePos = m_pWindow->getCursorPosition();
		return;
	}

	if (m_pWindow->getMouseButton(sa::MouseButton::BUTTON_2)) {
		sa::Vector2 mousePos = m_pWindow->getCursorPosition();
		sa::Vector2 delta = m_lastMousePos - mousePos;
		m_camera.rotate(glm::radians(delta.x) * dt * m_mouseSensitivity, { 0, 1, 0 });
		m_camera.rotate(glm::radians(delta.y) * dt * m_mouseSensitivity, m_camera.getRight());
		m_lastMousePos = mousePos;
	}
	else {
		m_lastMousePos = m_pWindow->getCursorPosition();
	}

	int forward = m_pWindow->getKey(sa::Key::W) - m_pWindow->getKey(sa::Key::S);
	int right = m_pWindow->getKey(sa::Key::D) - m_pWindow->getKey(sa::Key::A);
	int up = m_pWindow->getKey(sa::Key::Q) - m_pWindow->getKey(sa::Key::E);

	glm::vec3 camPos = m_camera.getPosition();
	camPos += (float)forward * dt * m_moveSpeed * m_camera.getForward();
	camPos += (float)right * dt * m_moveSpeed * m_camera.getRight();
	camPos += (float)up * dt * m_moveSpeed * m_camera.getUp();

	m_camera.setPosition(camPos);

}

void EditorView::onImGui() {
	SA_PROFILE_FUNCTION();

	if (ImGui::Begin("Editor view", 0, ImGuiWindowFlags_MenuBar)) {
		if (ImGui::BeginMenuBar()) {

			if (ImGui::BeginMenu("Menu")) {


				for (int i = 0; i < 5; i++) {
					ImGui::Button("something");
				}
				ImGui::EndMenu();
			}

			static bool showStats = false;
			ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() - ImGui::GetFontSize() * 8);
			ImGui::Checkbox("Statistics", &showStats);
			if (showStats) {
				ImGui::SetNextWindowBgAlpha(0.2f);
				if (ImGui::Begin("Statistics", 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize)) {

					ImGui::Text("Frame time: %f ms", m_statistics.frameTime * 1000);
					ImGui::Text("FPS: %f", 1 / m_statistics.frameTime);
					auto stats = sa::Renderer::get().getGPUMemoryUsage();
				
					ImGui::Text("GPU Memory usage");
					size_t totalUsage = 0;
					size_t totalBudget = 0;

					ImGui::Indent();
					for (auto& heap : stats.heaps) {
						ImGui::Text("%llu MB / %llu MB, flags: %u", heap.usage / 1000000, heap.budget / 1000000, heap.flags);
						totalUsage += heap.usage;
						totalBudget += heap.budget;
					}
					ImGui::Text("Total usage: %llu MB / %llu MB", totalUsage / 1000000, totalBudget / 1000000);
					ImGui::Unindent();
				}
				ImGui::End();
			}

		}
		ImGui::EndMenuBar();

		sa::Scene* pScene = m_pEngine->getCurrentScene();
		pScene->on<event::EntitySelected>([&](const event::EntitySelected& e, sa::Scene&) {
			m_selectedEntity = e.entity;
		});

		pScene->on<event::EntityDeselected>([&](const event::EntityDeselected&, sa::Scene&) {
			m_selectedEntity = {};
		});

		m_isFocused = ImGui::IsWindowFocused();

		// render outputTexture with constant aspect ratio
		ImVec2 availSize = ImGui::GetContentRegionAvail();

		float aspectRatio = (float)m_texture.getExtent().height / m_texture.getExtent().width;
		availSize.y = std::min(availSize.x * aspectRatio, (float)m_texture.getExtent().height);
		ImGui::Image(m_texture, availSize);
		m_displayedSize = availSize;

		const ImU32 red = ImColor(ImVec4(1, 0, 0, 1));
		const ImU32 green = ImColor(ImVec4(0, 1, 0, 1));
		const ImU32 blue = ImColor(ImVec4(0, 0, 1, 1));
		
		if (m_selectedEntity) {
			comp::Transform* transform = m_selectedEntity.getComponent<comp::Transform>();

			if (transform) {
				glm::vec3 pos = transform->position;

				glm::mat4 mat(1);
				mat = glm::rotate(mat, transform->rotation.x, glm::vec3(1, 0, 0));
				mat = glm::rotate(mat, transform->rotation.y, glm::vec3(0, 1, 0));
				mat = glm::rotate(mat, transform->rotation.z, glm::vec3(0, 0, 1));
				
				glm::vec3 x = glm::normalize(mat[0]);
				glm::vec3 y = glm::normalize(mat[1]);
				glm::vec3 z = glm::normalize(mat[2]);


				imGuiDrawVector(pos, x, red, 1);
				imGuiDrawVector(pos, y, green, 1);
				imGuiDrawVector(pos, z, blue, 1);
			}
		}
		
	}
	ImGui::End();


}

void EditorView::imGuiDrawLine(glm::vec3 p1, glm::vec3 p2, const ImColor& color, float thickness) {
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

void EditorView::imGuiDrawVector(glm::vec3 origin, glm::vec3 v, const ImColor& color, float thickness) {
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

	}
}

void EditorView::imGuiDrawVector(glm::vec3 v, const ImColor& color, float thickness) {
	imGuiDrawVector({ 0, 0, 0 }, v, color, thickness);
}


sa::Camera* EditorView::getCamera() {
	return &m_camera;
}

sa::Entity EditorView::getEntity() const {
	return m_selectedEntity;
}

void EditorView::setEntity(sa::Entity entity) {
	m_selectedEntity = entity;
}
