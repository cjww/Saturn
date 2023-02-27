#include "GameView.h"

#include "Tools\ScopeTimer.h"

GameView::GameView(sa::Engine* pEngine, sa::EngineEditor* pEditor, sa::RenderWindow* pWindow)
	: EditorModule(pEngine, pEditor, "Game View", true)
{
	m_isOpen = true;

	m_renderedCamera = false;
	
	m_resolutionIndex = 0;

	//m_renderTarget.initialize(m_Resolutions[0]);
	

	m_isWindowOpen = false;

	m_mipLevel = 0;

	pEngine->on<sa::engine_event::OnRender>([&](sa::engine_event::OnRender& e, sa::Engine& engine) {
		SA_PROFILE_SCOPE("GameView: OnRender");
		m_renderedCamera = false;
		if (!m_isWindowOpen || !m_isOpen)
			return;
		/*
		m_sceneCollection.clear();
		m_sceneCollection.collect(engine.getCurrentScene());
		m_sceneCollection.makeRenderReady();

		engine.getCurrentScene()->forEach<comp::Camera>([&](comp::Camera& camera) {
			if (camera.camera.getViewport().extent != m_renderTarget.extent) {
				camera.camera.setViewport({ { 0, 0 }, m_renderTarget.extent });
			}
			e.pRenderPipeline->render(*e.pContext, &camera.camera, &m_renderTarget, m_sceneCollection);
		});
		m_sceneCollection.swap();
		*/
			m_renderedCamera = true;
		

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
						//m_renderTarget.resize(m_Resolutions[i]);
					}
				}
				
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		if (m_renderedCamera && m_pEngine->getMainRenderTarget().isReady()) {
			// render outputTexture with constant aspect ratio
			sa::Extent extent = m_pEngine->getMainRenderTarget().extent;
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
			ImGui::Image((sa::Texture)*m_pEngine->getMainRenderTarget().outputTexture, {imageSize.x, imageSize.y});
			
		}
	}
	ImGui::End();

}
