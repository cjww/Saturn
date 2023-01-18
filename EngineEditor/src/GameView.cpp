#include "GameView.h"

#include "Tools\ScopeTimer.h"

GameView::GameView(sa::Engine* pEngine, sa::EngineEditor* pEditor, sa::RenderWindow* pWindow)
	: EditorModule(pEngine, pEditor)
{
	m_renderTarget.colorTexture = sa::DynamicTexture2D(sa::TextureTypeFlagBits::COLOR_ATTACHMENT | sa::TextureTypeFlagBits::SAMPLED, pWindow->getCurrentExtent());
	m_renderTarget.framebuffer = pEngine->getRenderPipeline().getRenderTechnique()->createColorFramebuffer(m_renderTarget.colorTexture);

	m_renderedCamera = false;
	
	m_resolutionIndex = 0;

	m_Resolutions[0] = sa::Renderer::get().getFramebufferExtent(m_renderTarget.framebuffer);

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
				e.pRenderPipeline->render(&camera.camera, &m_renderTarget);
				sa::Renderer::get().swapFramebuffer(m_renderTarget.framebuffer);
				m_renderedCamera = true;
			}
		});
		
	});

	pEngine->on<sa::engine_event::WindowResized>([&](sa::engine_event::WindowResized& e, sa::Engine& engine) {
		
		sa::Renderer::get().destroyFramebuffer(m_renderTarget.framebuffer);
		m_renderTarget.colorTexture.destroy();

		m_renderTarget.colorTexture = sa::DynamicTexture2D(sa::TextureTypeFlagBits::COLOR_ATTACHMENT | sa::TextureTypeFlagBits::SAMPLED, e.newExtent);
		m_renderTarget.framebuffer = pEngine->getRenderPipeline().getRenderTechnique()->createColorFramebuffer(m_renderTarget.colorTexture);

		m_Resolutions[0] = sa::Renderer::get().getFramebufferExtent(m_renderTarget.framebuffer);

		m_mipLevel = 0;
	});
}

void GameView::update(float dt) {

}

void GameView::onImGui() {

	if (m_isWindowOpen = ImGui::Begin("Game View", 0, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_MenuBar)) {
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

		if (m_renderedCamera && m_renderTarget.outputTexture) {
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
			ImGui::Image((sa::Texture)*m_renderTarget.outputTexture, { imageSize.x, imageSize.y });
			
		}
	}
	ImGui::End();

}
