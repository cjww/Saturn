#pragma once

#include "Tools\Logger.hpp"
#include "Resources/ResourceManager.hpp"
#include "RenderProgramFactory.hpp"
/*
#include "imgui_impl_vulkan.h"
#include "imgui_impl_glfw.h"
*/

#include "Texture.hpp"

struct GLFWwindow;

namespace sa {

	class VulkanCore;
	class CommandBufferSet;

	class Swapchain;
	class RenderProgram;
	class FramebufferSet;


	class Renderer {
	protected:
		//VulkanCore* m_pCore;

		std::unique_ptr<VulkanCore> m_pCore;


		CommandBufferSet* m_pCurrentCommandBufferSet;

		/*
		CommandBufferSet* m_pComputeCommandBufferSet;
		*/


		//DeviceMemoryManager* m_pDataManager;

		/*
		
		std::unordered_map<VkCommandBuffer, VkFence> m_transferCommandBuffers;
		std::vector<TransferCommand> m_transferCommandQueue;

		std::vector<VkSemaphore> m_imageAvailableSemaphore;
		std::vector<VkSemaphore> m_renderFinishedSemaphore;
		std::vector<VkFence> m_inFlightFences;
		std::vector<VkFence> m_imageFences;
		uint32_t m_frameIndex;

		// RESOURCES
		std::vector<RenderPass> m_renderPasses;
		std::vector<Framebuffer> m_framebuffers;
		std::vector<Pipeline> m_pipelines;

		std::vector<Swapchain> m_swapchains;
		std::unordered_map<uint32_t, ResizeCallbackFunc> m_swapchainResizeCallbacks;

		VkDescriptorPool m_imGuiDescriptorPool;
		std::unordered_map<Texture*, ImTextureID> m_imGuiImages;


		
		void createCommandPools();
		void createCommandBuffers();
		void createGraphicsCommandBuffers();

		void createSyncronisationObjects();

		VkSurfaceKHR createSurface(GLFWwindow* window);

		VkFramebuffer createFramebuffer(VkExtent2D extent, VkRenderPass renderPass, const std::vector<VkImageView>& imageViews);

		uint32_t createGraphicsPipeline(VkExtent2D extent, uint32_t renderPass, uint32_t subpassIndex,
			const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,
			const std::vector<VkPushConstantRange>& pushConstantRanges,
			const std::vector<VkPipelineShaderStageCreateInfo>& shaderStages,
			VkPipelineVertexInputStateCreateInfo vertexInput,
			vbl::PipelineConfig config = {});

		uint32_t createComputePipeline(const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,
			const std::vector<VkPushConstantRange>& pushConstantRanges,
			VkPipelineShaderStageCreateInfo shaderStage);

		void destroyFramebuffers();
		void destroyRenderPasses();
		void destroyPipelines();

		void destroySwapchains();

		void destroySyncronisationObjects();
		void freeGraphicsCommandBuffers();

		VkCommandBuffer beginTransferCommand(const Framebuffer& framebuffer, const RenderPass& renderPass, uint32_t subpass);
		void endTransferCommand(const TransferCommand& command);


		void createImGUIDescriptorPool();
		ImGui_ImplVulkan_InitInfo getImGUIInitInfo() const;

		*/

		friend class RenderProgramFactory;
		Swapchain* getSwapchain(ResourceID id);
		RenderProgram* getRenderProgram(ResourceID id);
		FramebufferSet* getFramebufferSet(ResourceID id);


		Renderer();
	public:
		static Renderer& get();
		virtual ~Renderer();

		ResourceID createSwapchain(GLFWwindow* pWindow);
		void destroySwapchain(ResourceID id);

		RenderProgramFactory createRenderProgram();
		ResourceID createFramebuffer(ResourceID renderProgram, const std::vector<Texture2D>& attachmentTextures, uint32_t layers = 1U);
		ResourceID createSwapchainFramebuffer(ResourceID swapchain, ResourceID renderProgram, const std::vector<Texture2D>& additionalAttachmentTextures, uint32_t layers = 1U);

		//RenderProgram features
		void beginRenderProgram(ResourceID renderProgram, ResourceID framebuffer, Color clearColor = { 0.f, 0.f, 0.f, 1.f }, Rect renderArea = { {0, 0}, {0, 0} });
		void endRenderProgram(ResourceID renderProgram);


		bool beginFrame(ResourceID swapchain);
		void endFrame(ResourceID swapchain);





		/*
		void initImGUI(GLFWwindow* window, uint32_t renderpass, uint32_t subpass);
		void newFrameImGUI();
		void endFrameImGUI();
		void cleanupImGUI();

		ImTextureID getImTextureID(Texture* texture, const SamplerPtr& sampler);

		uint32_t createSwapchain(GLFWwindow* window);
		void recreateSwapchain(uint32_t swapchain);

		void setOnSwapchainResizeCallback(uint32_t swapchain, ResizeCallbackFunc function);
		void invokeSwapchainResize(uint32_t swapchain);

		uint32_t getNextSwapchainImage(uint32_t swapchain);
		// create renderpasses, framebuffers and pipelines

		uint32_t createRenderPass(const std::vector<VkAttachmentDescription>& attachments,
			const std::vector<VkSubpassDescription>& subpasses,
			const std::vector<VkSubpassDependency>& dependencies);

		VkAttachmentDescription getSwapchainAttachment(uint32_t swapchain) const;

		uint32_t createSwapchainFramebuffer(uint32_t swapchain, uint32_t renderPass, const std::vector<Texture*>& additionalAttachments);
		uint32_t createFramebuffer(uint32_t renderPass, VkExtent2D extent, const std::vector<Texture*>& attachments);

		uint32_t createPipeline(const ShaderSetPtr& shaderSet, VkExtent2D extent = { 0, 0 }, uint32_t renderPass = 0, uint32_t subpassIndex = 0, vbl::PipelineConfig config = {});
		uint32_t createPipeline(uint32_t swapchain, const ShaderSetPtr& shaderSet, uint32_t renderPass = 0, uint32_t subpassIndex = 0, vbl::PipelineConfig config = {});

		//Resource creation
		Texture* createDepthTexture(VkExtent2D extent);


		Texture* createColorTexture2D(VkExtent2D extent, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT, uint32_t mipLevels = 1, uint32_t arrayLayers = 1);
		Texture* createTexture2D(uint32_t framebuffer, uint32_t renderpass, uint32_t subpass, VkExtent2D extent, unsigned char* pixels, int channels = 4);
		Texture* createColorAttachmentTexture(VkExtent2D extent, VkFormat format, uint32_t arrayLayers, uint32_t mipLevels, VkSampleCountFlagBits sampleCount, VkImageUsageFlags additionalUsage);

		Texture* createTexture3D(VkExtent3D extent, VkFormat format);

		void updateTexture(Texture* dst, uint32_t framebuffer, uint32_t renderpass, uint32_t subpass, void* data, size_t size);

		void queueTransferCommand(uint32_t framebuffer, uint32_t renderpass, uint32_t subpass, Buffer* srcBuffer, Texture* dstTexture);
	


		SamplerPtr createSampler(VkFilter minMagFilter, float maxAnisotropy = 16.0f, float minLod = 0.0f, float maxLod = 0.0f, float mipLodBias = 0.0f);
		SamplerPtr createSampler(VkSamplerCreateInfo info);


		Buffer* createVertexBuffer(VkDeviceSize size, void* initialData);
		Buffer* createIndexBuffer(VkDeviceSize size, void* initialData);

		Buffer* createUniformBuffer(VkDeviceSize size, void* initialData);
		Buffer* createStorageBuffer(VkDeviceSize size, void* initialData);

		void destroyBuffer(Buffer* buffer);
		void destroyTexture(Texture* texture);

		ShaderPtr createShader(const char* path, VkShaderStageFlagBits stage);
		ShaderSetPtr createShaderSet(const ShaderPtr& vertexShader, const ShaderPtr& fragmentShader);
		ShaderSetPtr createShaderSet(const ShaderPtr& vertexShader, const ShaderPtr& geometryShader, const ShaderPtr& fragmentShader);
		ShaderSetPtr createShaderSet(const ShaderPtr& computeShader);


		CommandBufferPtr createCommandBuffer(bool isCompute = false);
		void recordCommandBuffer(const CommandBufferPtr& commandBuffer, uint32_t framebuffer, uint32_t renderPass, uint32_t subpassIndex, std::function<void(uint32_t frameIndex)> drawCalls, bool isOneTimeRecord = false);
		void recordCommandBuffer(const CommandBufferPtr& commandBuffer, std::function<void(uint32_t frameIndex)> drawCalls, bool isOneTimeRecord = false);
		void executeCommandBuffer(const CommandBufferPtr& commandBuffer, bool isCompute = false);
		void executeCommandBuffers(const std::vector<CommandBufferPtr>& commandBuffers, bool isCompute = false);

		std::shared_ptr<VkFence> submitToComputeQueue(const CommandBufferPtr& commandBuffer);
		void waitForFence(std::shared_ptr<VkFence> fence, uint64_t timeout = UINT64_MAX);

		void waitDeviceIdle() const;

		void updateDescriptorSet(const DescriptorSetPtr& descriptorSet, uint32_t binding, const Buffer* buffer, const Texture* image, const SamplerPtr& sampler, bool isOneTimeUpdate);

		// Draw commands
		void beginRenderPass(uint32_t renderPass, uint32_t framebuffer, VkSubpassContents contents, glm::vec3 clearColor = glm::vec3(0.0f));
		void nextSubpass(VkSubpassContents contents);
		void endRenderPass();

		void transferTextureLayout(Texture* texture, VkImageLayout newLayout, const CommandBufferPtr& commandBuffer = nullptr, uint32_t frameIndex = -1);

		void bindPipeline(uint32_t pipeline, const CommandBufferPtr& commandBuffer = nullptr, uint32_t frameIndex = -1);
		void bindVertexBuffer(const Buffer* vertexBuffer, const CommandBufferPtr& commandBuffer = nullptr, uint32_t frameIndex = -1);
		void bindVertexBuffers(const std::vector<Buffer*>& vertexBuffers, const CommandBufferPtr& commandBuffer = nullptr, uint32_t frameIndex = -1);

		void bindIndexBuffer(const Buffer* indexbuffer, const CommandBufferPtr& commandBuffer = nullptr, uint32_t frameIndex = -1);

		void bindViewports(const std::vector<VkViewport>& viewports, const CommandBufferPtr& commandBuffer = nullptr, uint32_t frameIndex = -1);
		void bindViewport(const VkViewport& viewport, const CommandBufferPtr& commandBuffer = nullptr, uint32_t frameIndex = -1);

		void bindDescriptorSet(const DescriptorSetPtr& descriptorSet, uint32_t pipeline, const CommandBufferPtr& commandBuffer = nullptr, uint32_t frameIndex = -1);
		void pushConstants(uint32_t pipeline, VkShaderStageFlags shaderStages, uint32_t offset, uint32_t size, void* data, const CommandBufferPtr& commandBuffer = nullptr, uint32_t frameIndex = -1);

		void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex = 0U, uint32_t firstInstance = 0U, const CommandBufferPtr& commandBuffer = nullptr, uint32_t frameIndex = -1);
		void drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex = 0U, uint32_t firstInstance = 0U, uint32_t vertexOffset = 0U, const CommandBufferPtr& commandBuffer = nullptr, uint32_t frameIndex = -1);
		void dispatchCompute(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ, const CommandBufferPtr& commandBuffer = nullptr, uint32_t frameIndex = -1);
		*/

	};
}
