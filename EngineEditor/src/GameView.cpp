#include "GameView.h"

GameView::GameView(sa::Engine* pEngine, sa::EngineEditor* pEditor, sa::RenderWindow* pWindow)
	: EditorModule(pEngine, pEditor)
{
	m_colorTexture = sa::Renderer::get().createTexture2D(sa::TextureTypeFlagBits::COLOR_ATTACHMENT | sa::TextureTypeFlagBits::SAMPLED, pWindow->getCurrentExtent());
	m_renderTarget.framebuffer = pEngine->getRenderPipeline().getRenderTechnique()->createColorFramebuffer(m_colorTexture);

	m_camera.setViewport(sa::Rect{ { 0, 0 }, m_colorTexture.getExtent() });

	pEngine->on<sa::engine_event::OnRender>([&](sa::engine_event::OnRender& e, sa::Engine& engine) {
		e.pRenderPipeline->render(&m_camera, &m_renderTarget);
	});

	pEngine->on<sa::engine_event::WindowResized>([&](sa::engine_event::WindowResized& e, sa::Engine& engine) {
		m_colorTexture.destroy();
		m_colorTexture = sa::Renderer::get().createTexture2D(sa::TextureTypeFlagBits::COLOR_ATTACHMENT | sa::TextureTypeFlagBits::SAMPLED, e.newExtent);

		m_camera.setViewport(sa::Rect{ { 0, 0 }, m_colorTexture.getExtent() });

		sa::Renderer::get().destroyFramebuffer(m_renderTarget.framebuffer);
		m_renderTarget.framebuffer = engine.getRenderPipeline().getRenderTechnique()->createColorFramebuffer(m_colorTexture);
	});
}

void GameView::update(float dt) {

}

void GameView::onImGui() {

	if (ImGui::Begin("Game View")) {
		// render outputTexture with constant aspect ratio
		ImVec2 imAvailSize = ImGui::GetContentRegionAvail();
		glm::vec2 imageSize(imAvailSize.x, imAvailSize.y);

		float aspect = (float)m_colorTexture.getExtent().height / m_colorTexture.getExtent().width;
		imageSize.y = imAvailSize.x * aspect;

		if (imageSize.y >= imAvailSize.y) {
			imageSize.y = imAvailSize.y;
			imageSize.x = imAvailSize.y / aspect;
		}

		ImGui::Image(m_colorTexture, { imageSize.x, imageSize.y });
	}
	ImGui::End();
}
