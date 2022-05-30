#pragma once

#include "structs.hpp"
#include "Resources/Buffer.hpp"

namespace vk {
	class Sampler;
	class Queue;
	class Fence;
}

namespace sa {
	class CommandBufferSet;
	class VulkanCore;
	
	class Swapchain;
	class RenderProgram;
	class FramebufferSet;
	class Pipeline;
	class DescriptorSet;


	typedef uint32_t ShaderStageFlags;

	enum ShaderStageFlagBits : ShaderStageFlags {
		VERTEX = 1,
		TESSELATION_CONTROL = 2,
		TESSELATION_EVALUATION = 4,
		GEOMETRY = 8,
		FRAGMENT = 16,
		COMPUTE = 32,
	};

	
	class RenderContext {
	protected:
		CommandBufferSet* m_pCommandBufferSet;
		VulkanCore* m_pCore;

		friend class RenderProgramFactory;
		friend class Renderer;
		static Swapchain* getSwapchain(ResourceID id);
		static RenderProgram* getRenderProgram(ResourceID id);
		static FramebufferSet* getFramebufferSet(ResourceID id);
		static Pipeline* getPipeline(ResourceID id);
		static DescriptorSet* getDescriptorSet(ResourceID id);
		static vk::Sampler* getSampler(ResourceID id);

	public:
		RenderContext();

		RenderContext(VulkanCore* pCore, CommandBufferSet* pCommandBufferSet);

		void beginRenderProgram(ResourceID renderProgram, ResourceID framebuffer, Rect renderArea = { {0, 0}, {0, 0} });
		void nextSubpass();
		void endRenderProgram(ResourceID renderProgram);

		void bindPipeline(ResourceID pipeline);
		void bindVertexBuffers(uint32_t firstBinding, const std::vector<Buffer>& buffers);
		void bindIndexBuffer(const Buffer& buffer);

		void updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const Buffer& buffer);
		void bindDescriptorSets(const std::vector<ResourceID>& descriptorSets, ResourceID pipeline);
		void bindDescriptorSet(ResourceID descriptorSet, ResourceID pipeline);

		void pushConstants(ResourceID pipeline, ShaderStageFlags stages, uint32_t offset, size_t size, void* data);

		template<typename T>
		void pushConstants(ResourceID pipeline, ShaderStageFlags stages, uint32_t offset, const std::vector<T>& values);
		
		template<typename T>
		void pushConstant(ResourceID pipeline, ShaderStageFlags stages, uint32_t offset, const T& value);

		void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex = 0, uint32_t firstInstance = 0);
		void drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex = 0, uint32_t vertexOffset = 0, uint32_t firstInstance = 0);

		void dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);

		operator bool() {
			return m_pCommandBufferSet != nullptr;
		}
	};

	class Context : public RenderContext {
	private:
		std::shared_ptr<vk::Fence> m_pFence;
		ResourceID m_commandBufferSetID;
		uint32_t m_bufferIndex;
	public:
		Context(VulkanCore* pCore, ResourceID commandBufferSetID);
		void begin();
		void end();

		void submit();

		void waitToFinish(size_t timeout = UINT64_MAX);

		void destroy();
	};


	template<typename T>
	inline void RenderContext::pushConstants(ResourceID pipeline, ShaderStageFlags stages, uint32_t offset, const std::vector<T>& values) {
		pushConstants(pipeline, stages, offset, values.size() * sizeof(T), (void*)values.data());
	}

	template<typename T>
	inline void RenderContext::pushConstant(ResourceID pipeline, ShaderStageFlags stages, uint32_t offset, const T& value) {
		pushConstants(pipeline, stages, offset, sizeof(T), (void*)&value);
	}

}