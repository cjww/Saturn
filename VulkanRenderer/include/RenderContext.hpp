#pragma once
#include "imgui.h"

#include "structs.hpp"
#include "Resources/Buffer.hpp"
#include "Resources/Texture.hpp"
#include "Resources/ImageTransitions.hpp"
#include "Shader.hpp"

namespace vk {
	class Sampler;
	class Queue;
	class Fence;
	class Pipeline;
	class PipelineLayout;
}

namespace sa {
	class CommandBufferSet;
	class VulkanCore;
	
	class Swapchain;
	class RenderProgram;
	class FramebufferSet;
	class Pipeline;
	class DescriptorSet;
	class ShaderModule;


	
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

	class SubContext;

	struct DrawIndexedIndirectCommand {
		uint32_t indexCount = 0U;
		uint32_t instanceCount = 0U;
		uint32_t firstIndex = 0U;
		int32_t  vertexOffset = 0;
		uint32_t firstInstance = 0U;
	};

	struct DrawIndirectCommand {
		uint32_t vertexCount = 0U;
		uint32_t instanceCount = 0U;
		uint32_t firstVertex = 0U;
		uint32_t firstInstance = 0U;
	};

	class RenderContext {
	protected:
		CommandBufferSet* m_pCommandBufferSet;
		VulkanCore* m_pCore;

		PipelineLayout* m_pLastPipelineLayout;

		friend class RenderProgramFactory;
		friend class Renderer;
		static Swapchain* GetSwapchain(ResourceID id);
		static RenderProgram* GetRenderProgram(ResourceID id);
		static FramebufferSet* GetFramebufferSet(ResourceID id);
		static vk::Pipeline* GetPipeline(ResourceID id);
		static DescriptorSet* GetDescriptorSet(ResourceID id);
		static vk::Sampler* GetSampler(ResourceID id);
		static vk::PipelineLayout* GetPipelineLayout(ResourceID id);

		void bindVertexInput(const PipelineLayout& layout) const;

	public:
		RenderContext();

		RenderContext(VulkanCore* pCore, CommandBufferSet* pCommandBufferSet);
#ifndef IMGUI_DISABLE
		void renderImGuiFrame() const;
#endif

		void beginRenderProgram(ResourceID renderProgram, ResourceID framebuffer, SubpassContents contents, Rect renderArea = { {0, 0}, {0, 0} }) const;
		void nextSubpass(SubpassContents contentType) const;
		void endRenderProgram(ResourceID renderProgram) const;

		void beginRendering(const std::vector<Texture>& colorAttachments, const std::vector<Texture>& depthAttachments);

		void executeSubContext(const sa::SubContext& context) const;

		void bindPipelineLayout(const PipelineLayout& pipelineLayout);
		void bindPipeline(ResourceID pipeline) const;
		void bindShader(const Shader& shader) const;
		void bindShaders(const std::vector<Shader>& shaders) const;
		void bindVertexBuffers(uint32_t firstBinding, const Buffer* pBuffers, size_t bufferCount) const;
		void bindIndexBuffer(const Buffer& buffer) const;

		void updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const Buffer& buffer) const;
		
		void updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const Texture& texture, ResourceID sampler) const;
		void updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const Texture& texture) const;

		void updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const std::vector<Texture>& textures, uint32_t firstElement) const;
		void updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const std::vector<Texture>& textures, ResourceID sampler, uint32_t firstElement) const;

		void updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const Texture* textures, uint32_t textureCount, ResourceID sampler, uint32_t firstElement) const;
		void updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const Texture* textures, uint32_t textureCount, uint32_t firstElement) const;

		void updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, ResourceID sampler) const;


		void bindDescriptorSets(const std::vector<ResourceID>& descriptorSets) const;
		void bindDescriptorSet(ResourceID descriptorSet) const;

		void pushConstants(ShaderStageFlags stages, uint32_t offset, uint32_t size, const void* data) const;

		template<typename T>
		void pushConstants(ShaderStageFlags stages, const std::vector<T>& values, uint32_t offset = UINT32_MAX);
		
		template<typename T>
		void pushConstant(ShaderStageFlags stages, const T& value, uint32_t offset = UINT32_MAX);

		void setScissor(Rect scissor) const;
		void setViewport(Rect viewport) const;
		void setViewports(const Rect* pViewports, uint32_t viewportCount) const;

		void setDepthBias(float constantFactor, float clamp, float slopeFactor) const;
		void setDepthBiasEnable(bool enable) const;
		void setPrimitiveTopology(Topology topology) const;
		void setPatchControlPoints(uint32_t points) const;
		void setPrimitiveRestartEnable(bool enable) const;
		void setRasterizerDiscardEnable(bool enable) const;

		void setCullMode(CullModeFlags cullMode) const;
		/*
		void setRasterizerDiscardEnable() const;
		void setColorBlend() const;
		void setTessellationDomainOrigin() const;

		void setRasterizationSamples() const;
		void setSampleMask() const;
		void setAlphaToCoverageEnable() const;
		void setAlphaToOneEnable() const;
		void setPolygonMode() const;
		void setLineWidth() const;
		void setCullMode() const;
		void setFrontFace() const;
		void setDepthTestEnable() const;
		void setDepthWriteEnable() const;
		void setDepthCompareOp() const;
		void setDepthBoundsTestEnable() const;
		void setDepthBounds() const;
		void setDepthClampEnableEXT() const;
		void setStencilTestEnable() const;
		void setStencilOp() const;
		void setStencilCompareMask() const;
		void setStencilWriteMask() const;
		void setStencilReference() const;

		void setLogicOpEnable() const;
		void setLogicOp() const;
		void setColorBlendEnable() const;
		void setColorWriteMask() const;
		void setColorBlendEquation() const;
		void setColorBlendAdvanced() const;
		void setBlendConstants() const;
		 */
		/*
		void setFragmentShadingRate() const;
		void setRasterizationStream() const;

		void setDiscardRectangleEnable() const;
		void setDiscardRectangleMode() const;
		void setDiscardRectangle() const;

		void setConservativeRasterizationMode() const;
		void setExtraPrimitiveOverestimationSize() const;

		void setDepthClipEnable() const;
		*/

		void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex = 0, uint32_t firstInstance = 0) const;
		void drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex = 0, uint32_t vertexOffset = 0, uint32_t firstInstance = 0) const;
		void drawIndexedIndirect(const Buffer& buffer, uint32_t offset, uint32_t drawCount, uint32_t stride) const;
		void drawIndexedIndirect(const Buffer& buffer, size_t offset, const Buffer& countBuffer, size_t countOffset, uint32_t stride, uint32_t maxDrawCount = 0) const;
		void drawIndirect(const Buffer& buffer, uint32_t offset, uint32_t drawCount, uint32_t stride) const;
		void drawIndirect(const Buffer& buffer, size_t offset, const Buffer& countBuffer, size_t countOffset, uint32_t stride, uint32_t maxDrawCount = 0) const;

		void dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) const;

		void barrierDepthAttachmentToCompute(const Texture& texture) const;
		void barrierComputeToFragment(const Texture& texture) const;

		void barrierColorCompute(const Texture& texture) const;
		void barrierColorCompute(const Buffer& buffer) const;

		void barrier(const Texture& texture, Transition src, Transition dst) const;
		void barrier(uint32_t textureCount, const Texture* pTextures, Transition src, Transition dst) const;
		void barrier(const Buffer& buffer, Transition src, Transition dst) const;

		void barrier(Transition src, Transition dst) const;
		void fullBarrier() const;



		void copyImageToImageColor(const Texture& src, const Texture& dst) const;
		void copyImageToSwapchain(const Texture& src, ResourceID swapchain) const;


		uint32_t getFrameIndex() const;
		void syncFramebuffer(ResourceID framebuffer);

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
	inline void RenderContext::pushConstants(ShaderStageFlags stages, const std::vector<T>& values, uint32_t offset) {
		pushConstants(stages, offset, values.size() * sizeof(T), (void*)values.data());
	}

	template<typename T>
	inline void RenderContext::pushConstant(ShaderStageFlags stages, const T& value, uint32_t offset) {
		pushConstants(stages, offset, sizeof(T), (void*)&value);
	}

}