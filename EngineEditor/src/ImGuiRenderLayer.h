#pragma once
#include <Graphics/WindowRenderer.h>
#include <RenderWindow.hpp>

class ImGuiRenderLayer : public sa::IWindowRenderer {
private:
	
	sa::Renderer& m_renderer;
	sa::RenderWindow* m_pWindow;

	ResourceID m_imGuiRenderProgram = NULL_RESOURCE;
	ResourceID m_imGuiFramebuffer = NULL_RESOURCE;
	

public:
	
	ImGuiRenderLayer(sa::RenderWindow* pWindow);

	virtual void init();
	virtual void cleanup();

	virtual void render(sa::RenderContext& context, const sa::Texture& texture) override;

	virtual void onWindowResize(sa::Extent newExtent) override;

};


