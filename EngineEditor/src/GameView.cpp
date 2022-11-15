#include "GameView.h"

GameView::GameView(sa::Engine* pEngine, sa::EngineEditor* pEditor, sa::RenderWindow* pWindow)
	: EditorModule(pEngine, pEditor)
{
	m_colorTexture = sa::Renderer::get().createTexture2D(sa::TextureTypeFlagBits::COLOR_ATTACHMENT | sa::TextureTypeFlagBits::SAMPLED, pWindow->getCurrentExtent());
	m_renderTarget.framebuffer = pEngine->getRenderPipeline().getRenderTechnique()->createColorFramebuffer(m_colorTexture);

	m_renderedCamera = false;
	
	m_resolutionIndex = 0;

	m_Resolutions[0] = m_colorTexture.getExtent();

	pEngine->on<sa::engine_event::OnRender>([&](sa::engine_event::OnRender& e, sa::Engine& engine) {
		m_renderedCamera = false;
		engine.getCurrentScene()->forEach<comp::Camera>([&](comp::Camera& camera) {
			if (camera.isPrimary) {
				e.pRenderPipeline->render(&camera.camera, &m_renderTarget);
				m_renderedCamera = true;
			}
		});
	});

	pEngine->on<sa::engine_event::WindowResized>([&](sa::engine_event::WindowResized& e, sa::Engine& engine) {
		m_colorTexture.destroy();
		m_colorTexture = sa::Renderer::get().createTexture2D(sa::TextureTypeFlagBits::COLOR_ATTACHMENT | sa::TextureTypeFlagBits::SAMPLED, e.newExtent);
		
		m_Resolutions[0] = m_colorTexture.getExtent();

		sa::Renderer::get().destroyFramebuffer(m_renderTarget.framebuffer);
		m_renderTarget.framebuffer = engine.getRenderPipeline().getRenderTechnique()->createColorFramebuffer(m_colorTexture);
	});
}

void GameView::update(float dt) {

}

void GameView::onImGui() {

	if (ImGui::Begin("Game View", 0, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_MenuBar)) {
		if (ImGui::BeginMenuBar()) {

			if (ImGui::BeginMenu("Format")) {
				ImGui::RadioButton("Window Size", &m_resolutionIndex, 0);
				ImGui::RadioButton("1920x1080", &m_resolutionIndex, 1);
				ImGui::RadioButton("1366x768", &m_resolutionIndex, 2);
				ImGui::RadioButton("1600x900", &m_resolutionIndex, 3);
				ImGui::RadioButton("1920x1200", &m_resolutionIndex, 4);
				ImGui::RadioButton("2560x1440", &m_resolutionIndex, 5);
				ImGui::RadioButton("3440x1440", &m_resolutionIndex, 6);
				
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		if (m_renderedCamera) {
			// render outputTexture with constant aspect ratio

			sa::Extent extent = m_Resolutions[m_resolutionIndex];
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
			ImGui::Image(m_colorTexture, { imageSize.x, imageSize.y });
			
		}
	}
	ImGui::End();
}
