#pragma once

#include <Renderer.hpp>
#include <structs.hpp>
#include <Resources\Texture.hpp>
#include <RenderWindow.hpp>

namespace sa {
	class IWindowRenderer {
	public:
		virtual ~IWindowRenderer();
		virtual void render(RenderContext& context, const Texture& texture) = 0;		
		virtual void onWindowResize(Extent newExtent) = 0;
	};

	class WindowRenderer : public IWindowRenderer {
	private:
		RenderWindow* m_pWindow;

		ResourceID m_swapchainFramebuffer;

		Shader m_vertexShader;
		Shader m_fragmentShader;
		PipelineLayout m_swapchainPipelineLayout;
		ResourceID m_swapchainPipeline;
		
		ResourceID m_swapchainRenderProgram;
		ResourceID m_swapchainDescriptorSet;
		ResourceID m_sampler;


		

	public:		
		WindowRenderer(RenderWindow* pWindow);

		virtual void render(RenderContext& context, const Texture& texture) override;
		void onWindowResize(Extent newExtent) override;

	};

}