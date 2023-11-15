#include "GameView.h"

#include "Tools\ScopeTimer.h"

GameView::GameView(sa::Engine* pEngine, sa::EngineEditor* pEditor, sa::RenderWindow* pWindow)
	: EditorModule(pEngine, pEditor, "Game View", true)
{
	m_isOpen = true;

	m_resolutionIndex = 0;

	//m_renderTarget.initialize(m_Resolutions[0]);

	m_isWindowOpen = false;

	m_mipLevel = 0;

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

		if (m_pEngine->getMainRenderTarget().isReady()) {
			// render outputTexture with constant aspect ratio
			sa::Extent extent = m_pEngine->getMainRenderTarget().getExtent();
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
			ImGui::Image(m_pEngine->getMainRenderTarget().getOutputTexture(), {imageSize.x, imageSize.y});
			
		}
	}
	ImGui::End();

}
