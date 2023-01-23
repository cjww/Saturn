#include "GameView.h"

#include "Tools\ScopeTimer.h"

GameView::GameView(sa::Engine* pEngine, sa::EngineEditor* pEditor, sa::RenderWindow* pWindow)
	: EditorModule(pEngine, pEditor, "Game View", true)
{
	m_isOpen = true;

	m_renderedCamera = false;
	
	m_resolutionIndex = 0;

	m_renderTarget.initialize(pEngine, m_Resolutions[0]);
	

	m_isWindowOpen = false;

	m_mipLevel = 0;

	pEngine->on<sa::engine_event::OnRender>([&](sa::engine_event::OnRender& e, sa::Engine& engine) {
		SA_PROFILE_SCOPE("GameView: OnRender");
		m_renderedCamera = false;
		if (!m_isWindowOpen)
			return;
		
		engine.getCurrentScene()->forEach<comp::Camera, comp::Transform>([&](comp::Camera& camera, comp::Transform& transform) {
			camera.camera.setPosition(transform.position);
			glm::vec3 forward = transform.rotation * glm::vec3(0, 0, 1);
			camera.camera.lookAt(transform.position + forward);
		});

		engine.getCurrentScene()->forEach<comp::Camera>([&](comp::Camera& camera) {
			if (camera.isPrimary) {
				if (camera.camera.getViewport().extent != m_renderTarget.extent) {
					camera.camera.setViewport({ { 0, 0 }, m_renderTarget.extent });
				}
				e.pRenderPipeline->render(&camera.camera, &m_renderTarget);
				sa::Renderer::get().swapFramebuffer(m_renderTarget.framebuffer);
				m_renderedCamera = true;
			}
		});
		
	});
}

void GameView::update(float dt) {

}

void GameView::onImGui() {
	if (!m_isOpen) 
		return;

	if (m_isWindowOpen = ImGui::Begin(m_name, &m_isOpen, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_MenuBar)) {
		if (ImGui::BeginMenuBar()) {

			if (ImGui::BeginMenu("Format")) {
				std::string s;
				for (int i = 0; i < m_Resolutions.size(); i++) {
					s = std::to_string(m_Resolutions[i].width) + "x" + std::to_string(m_Resolutions[i].height);
					if (ImGui::RadioButton(s.c_str(), &m_resolutionIndex, i)) {
						SA_DEBUG_LOG_INFO("Changed game resolution: ", m_Resolutions[i].width, "x", m_Resolutions[i].height);
						m_renderTarget.resize(m_Resolutions[i]);
					}
				}
				
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		if (m_renderedCamera && m_renderTarget.outputTexture && m_renderTarget.isInitialized && m_renderTarget.outputTexture->isValid()) {
			// render outputTexture with constant aspect ratio
			sa::Extent extent = m_renderTarget.extent;
			float aspect = (float)extent.height / extent.width;
			
			ImVec2 imAvailSize = ImGui::GetContentRegionAvail();
			glm::vec2 imageSize(imAvailSize.x, imAvailSize.y);
			glm::vec2 pos = glm::vec2(ImGui::GetCursorPos().x, ImGui::GetCursorPos().y) + imageSize * 0.5f;

			imageSize.y = imAvailSize.x * aspect;

			if (imageSize.y >= imAvailSize.y) {
				imageSize.y = imAvailSize.y;
				imageSize.x = imAvailSize.y / aspect;
			}

			pos -= imageSize * 0.5f;
			
			ImGui::SetCursorPos({ pos.x, pos.y });
			ImGui::Image((sa::Texture)*m_renderTarget.outputTexture, { imageSize.x, imageSize.y });
			
		}
	}
	ImGui::End();

}
