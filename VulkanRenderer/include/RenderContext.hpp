#pragma once

#include "structs.hpp"
#include "Resources/Buffer.hpp"

namespace sa {
	class CommandBufferSet;
	
	class Swapchain;
	class RenderProgram;
	class FramebufferSet;
	class Pipeline;
	class DescriptorSet;

	class RenderContext {
	private:
		CommandBufferSet* m_pCommandBufferSet;
		
		friend class RenderProgramFactory;
		friend class Renderer;
		static Swapchain* getSwapchain(ResourceID id);
		static RenderProgram* getRenderProgram(ResourceID id);
		static FramebufferSet* getFramebufferSet(ResourceID id);
		static Pipeline* getPipeline(ResourceID id);
		static DescriptorSet* getDescriptorSet(ResourceID id);



	public:
		RenderContext();

		RenderContext(CommandBufferSet* pCommandBufferSet);

		void beginRenderProgram(ResourceID renderProgram, ResourceID framebuffer, Rect renderArea = { {0, 0}, {0, 0} });
		void endRenderProgram(ResourceID renderProgram);

		void bindPipeline(ResourceID pipeline);
		void bindVertexBuffers(uint32_t firstBinding, const std::vector<Buffer>& buffers);
		void bindIndexBuffer(const Buffer& buffer);

		void updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const Buffer& buffer);
		void bindDescriptorSets(const std::vector<ResourceID>& descriptorSets, ResourceID pipeline);
		void bindDescriptorSet(ResourceID descriptorSet, ResourceID pipeline);

		void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex = 0, uint32_t firstInstance = 0);
		void drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex = 0, uint32_t vertexOffset = 0, uint32_t firstInstance = 0);


		operator bool() {
			return m_pCommandBufferSet != nullptr;
		}
	};
}