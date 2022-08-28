#pragma once
#include "imgui.h"

#include "structs.hpp"
#include "Resources/Buffer.hpp"
#include "Resources\Texture.hpp"
#include "Resources/DynamicBuffer.h"
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

	typedef uint32_t ContextUsageFlags;

	enum ContextUsageFlagBits : ContextUsageFlags {
		ONE_TIME_SUBMIT = 1,
		RENDER_PROGRAM_CONTINUE = 2,
		SIMULTANEOUS_USE = 4
	};

	enum class SubpassContents {
		DIRECT,
		SUB_CONTEXT
	};

	enum class Transition {
		NONE,
		RENDER_PROGRAM_INPUT,
		RENDER_PROGRAM_OUTPUT,
		COMPUTE_SHADER_READ,
		COMPUTE_SHADER_WRITE,
		FRAGMENT_SHADER_READ,
		FRAGMENT_SHADER_WRITE,
	};

	class SubContext;

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
#ifndef IMGUI_DISABLE
		void renderImGuiFrame();
#endif

		void beginRenderProgram(ResourceID renderProgram, ResourceID framebuffer, SubpassContents contents, Rect renderArea = { {0, 0}, {0, 0} });
		void nextSubpass(SubpassContents contentType);
		void endRenderProgram(ResourceID renderProgram);

		void executeSubContext(const sa::SubContext& context);

		void bindPipeline(ResourceID pipeline);
		void bindVertexBuffers(uint32_t firstBinding, const std::vector<Buffer>& buffers);
		void bindIndexBuffer(const Buffer& buffer);

		void updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const Buffer& buffer);
		void updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, DynamicBuffer& buffer);

		void updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const Texture& texture, ResourceID sampler);
		void updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const Texture& texture);
		void updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const std::vector<Texture>& textures, uint32_t firstElement = 0);
		void updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, ResourceID sampler);


		void bindDescriptorSets(const std::vector<ResourceID>& descriptorSets, ResourceID pipeline);
		void bindDescriptorSet(ResourceID descriptorSet, ResourceID pipeline);

		void pushConstants(ResourceID pipeline, ShaderStageFlags stages, uint32_t offset, uint32_t size, void* data);

		template<typename T>
		void pushConstants(ResourceID pipeline, ShaderStageFlags stages, const std::vector<T>& values, uint32_t offset = UINT32_MAX);
		
		template<typename T>
		void pushConstant(ResourceID pipeline, ShaderStageFlags stages, const T& value, uint32_t offset = UINT32_MAX);


		void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex = 0, uint32_t firstInstance = 0);
		void drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex = 0, uint32_t vertexOffset = 0, uint32_t firstInstance = 0);

		void dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);

		void barrier(const Texture& texture);

		void transitionTexture(const Texture& texture, Transition src, Transition dst);


		operator bool() {
			return m_pCommandBufferSet != nullptr;
		}
	};

	class SubContext : public RenderContext {
	protected:
		ResourceID m_commandBufferSetID;
		
		FramebufferSet* m_pFramebufferSet;
		RenderProgram* m_pRenderProgram;
		uint32_t m_subpassIndex;

	public:
		SubContext();
		SubContext(VulkanCore* pCore, FramebufferSet* pFramebufferSet, RenderProgram* pRenderProgram, uint32_t subpassIndex, ResourceID contextPool);

		void begin(ContextUsageFlags usageFlags = 0);
		void end();
		void preRecord(std::function<void(RenderContext&)> function, ContextUsageFlags usageFlags = 0);

		void destroy();
	};

	class DirectContext : public RenderContext {
	private:
		std::shared_ptr<vk::Fence> m_pFence;
		ResourceID m_commandBufferSetID;

	public:
		DirectContext(VulkanCore* pCore, ResourceID contextPool);

		void begin(ContextUsageFlags usageFlags = 0);
		void end();

		void submit();
		void waitToFinish(size_t timeout = UINT64_MAX);

		void destroy();
	};

	template<typename T>
	inline void RenderContext::pushConstants(ResourceID pipeline, ShaderStageFlags stages, const std::vector<T>& values, uint32_t offset) {
		pushConstants(pipeline, stages, offset, values.size() * sizeof(T), (void*)values.data());
	}

	template<typename T>
	inline void RenderContext::pushConstant(ResourceID pipeline, ShaderStageFlags stages, const T& value, uint32_t offset) {
		pushConstants(pipeline, stages, offset, sizeof(T), (void*)&value);
	}

}