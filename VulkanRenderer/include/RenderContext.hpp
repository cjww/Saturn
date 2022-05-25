#pragma once

#include "structs.hpp"

namespace sa {
	class CommandBufferSet;
	
	class Swapchain;
	class RenderProgram;
	class FramebufferSet;
	class Pipeline;

	class RenderContext {
	private:
		CommandBufferSet* m_pCommandBufferSet;

		friend class RenderProgramFactory;
		friend class Renderer;
		static Swapchain* getSwapchain(ResourceID id);
		static RenderProgram* getRenderProgram(ResourceID id);
		static FramebufferSet* getFramebufferSet(ResourceID id);
		static Pipeline* getPipeline(ResourceID id);


	public:
		RenderContext();

		RenderContext(CommandBufferSet* pCommandBufferSet);

		void beginRenderProgram(ResourceID renderProgram, ResourceID framebuffer, Rect renderArea = { {0, 0}, {0, 0} });
		void endRenderProgram(ResourceID renderProgram);


		void bindPipeline(ResourceID pipeline);

		void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex = 0, uint32_t firstInstance = 0);


		operator bool() {
			return m_pCommandBufferSet != nullptr;
		}
	};
}