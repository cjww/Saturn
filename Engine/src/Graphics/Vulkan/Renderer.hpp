#pragma once
#include "common.hpp"

#include "ShaderSet.hpp"
#include "DataManager.hpp"
#include "Image.hpp"

#include <Tools/Logger.hpp>

#include <Graphics/RenderWindow.hpp>

#include "vulkan_base.hpp"

#include "functions.hpp"


#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "imgui_impl_vulkan.h"
#include "imgui_impl_glfw.h"


namespace NAME_SPACE {
	struct Swapchain {
		VkSwapchainKHR swapchain = VK_NULL_HANDLE;
		VkFormat format;
		std::vector<VkImage> images;
		std::vector<VkImageView> imageViews;
		VkExtent2D extent;
		uint32_t currentImageIndex;
	};

	struct RenderPass {
		VkRenderPass renderPass;
		uint32_t subpassCount;
		std::vector<uint32_t> colorAttachmentCount;
		std::vector<VkBool32> depthAttachment;
		std::vector<VkClearValue> clearValues;
	};

	struct Framebuffer {
		std::vector<VkFramebuffer> framebuffers;
	};

	struct CommandBuffer {
		std::vector<VkCommandBuffer> buffers;
	};

	struct TransferCommand {
		enum Type {
			BUFFER_TO_IMAGE,
			BUFFER_TO_BUFFER,
			IMAGE_TO_BUFFER,
			IMAGE_TO_IMAGE
		} type;
		Buffer* srcBuffer;
		Texture* srcImage;
		Buffer* dstBuffer;
		Texture* dstImage;
		VkCommandBuffer commandBuffer;
	};

	struct Pipeline {
		VkPipeline pipeline;
		VkPipelineLayout layout;
		bool isCompute;
	};

	typedef std::shared_ptr<CommandBuffer> CommandBufferPtr;
	typedef std::shared_ptr<VkSampler> SamplerPtr;

	class Renderer {
	private:
		static Renderer* m_pMyInstance;
	protected:		
		sa::RenderWindow* m_window;
		Swapchain m_swapchain;
		VkSurfaceKHR m_surface;
		uint32_t m_inFlightCount;

		VkApplicationInfo m_appInfo;
		VkInstance m_instance;
		VkDevice m_device;
		VkPhysicalDevice m_physicalDevice;

		std::vector<const char*> m_validationLayers;
		std::vector<const char*> m_instanceExtensions;
		std::vector<const char*> m_deviceExtensions;

		vbl::QueueInfo m_graphicsQueueInfo;
		VkQueue m_graphicsQueue;
		vbl::QueueInfo m_computeQueueInfo;
		VkQueue m_computeQueue;

		DataManager* m_pDataManager;

		VkCommandPool m_graphicsCommandPool;
		VkCommandPool m_computeCommandPool;
		std::vector<VkCommandBuffer> m_graphicsCommandBuffers;
		std::vector<VkCommandBuffer> m_computeCommandBuffers;
		std::vector<VkCommandBuffer> m_transferCommandBuffers;
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

		VkDescriptorPool m_imGuiDescriptorPool;
		std::unordered_map<Texture*, ImTextureID> m_imGuiImages;

		void setupDebug();

		void createInstance();
		void getPhysicalDevice();
		void createDevice();

		void createSurface(GLFWwindow* window);

		void createSwapchain();

		void createCommandPools();
		void createCommandBuffers();
		void createGraphicsCommandBuffers();

		void createSyncronisationObjects();

		VkFramebuffer createFramebuffer(VkExtent2D extent, VkRenderPass renderPass, const std::vector<VkImageView>& imageViews);

		uint32_t createGraphicsPipeline(uint32_t renderPass, uint32_t subpassIndex,
			const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,
			const std::vector<VkPushConstantRange>& pushConstantRanges,
			const std::vector<VkPipelineShaderStageCreateInfo>& shaderStages,
			VkPipelineVertexInputStateCreateInfo vertexInput,
			vbl::PipelineConfig config = {});

		uint32_t createComputePipeline(const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,
			const std::vector<VkPushConstantRange>& pushConstantRanges,
			VkPipelineShaderStageCreateInfo shaderStage);

		void destroyFramebuffers();
		void destroySwapchain();
		void destroySurface();
		void destroySyncronisationObjects();
		void freeGraphicsCommandBuffers();

		VkCommandBuffer beginTransferCommand(const Framebuffer& framebuffer, const RenderPass& renderPass, uint32_t subpass);
		void endTransferCommand(const TransferCommand& command);

		Renderer(sa::RenderWindow* window);

		void createImGUIDescriptorPool();
		ImGui_ImplVulkan_InitInfo getImGUIInitInfo() const;

	public:

		virtual ~Renderer();

		static void init(sa::RenderWindow* window);
		static Renderer* get();
		static void cleanup();

		void initImGUI(uint32_t renderpass, uint32_t subpass);
		void newFrameImGUI();
		void endFrameImGUI();
		void cleanupImGUI();

		ImTextureID getImTextureID(Texture* texture, const SamplerPtr& sampler);

		void createSwapchain(const sa::RenderWindow& window);
		void createSwapchain(VkSurfaceKHR surface);

		uint32_t getNextSwapchainImage();

		sa::RenderWindow* getWindow() const;

		bool beginFrame();
		void endFrame();
		void submit();
		void present();

		// create renderpasses, framebuffers and pipelines
		
		uint32_t createRenderPass(const std::vector<VkAttachmentDescription>& attachments,
			const std::vector<VkSubpassDescription>& subpasses,
			const std::vector<VkSubpassDependency>& dependencies);

		VkAttachmentDescription getSwapchainAttachment() const;

		uint32_t createSwapchainFramebuffer(uint32_t renderPass, const std::vector<Texture*>& additionalAttachments);
		uint32_t createFramebuffer(uint32_t renderPass, VkExtent2D extent, const std::vector<Texture*>& attachments);

		uint32_t createPipeline(const ShaderSetPtr& shaderSet, uint32_t renderPass = 0, uint32_t subpassIndex = 0, vbl::PipelineConfig config = {});

		//Resource creation
		Texture* createDepthTexture(VkExtent2D extent);

		Texture* createTexture2D(uint32_t framebuffer, uint32_t renderpass, uint32_t subpass, const Image& image);
		Texture* createTexture2D(uint32_t framebuffer, uint32_t renderpass, uint32_t subpass, VkExtent2D extent, unsigned char* pixels, int channels = 4);
		Texture* createColorAttachmentTexture(VkExtent2D extent, VkFormat format, uint32_t arrayLayers, uint32_t mipLevels, VkSampleCountFlagBits sampleCount, VkImageUsageFlags additionalUsage);

		Texture* createTexture3D(VkExtent3D extent, VkFormat format);

		void updateTexture(Texture* dst, uint32_t framebuffer, uint32_t renderpass, uint32_t subpass, void* data, size_t size);

		void queueTransferCommand(uint32_t framebuffer, uint32_t renderpass, uint32_t subpass, Buffer* srcBuffer, Texture* dstTexture);
		/*
		void queueTransferCommand(Buffer* srcBuffer, Buffer* dstBuffer);
		void queueTransferCommand(Texture* srcTexture, Buffer* dstBuffer);
		void queueTransferCommand(Texture* srcTexture, Texture* dstTexture);
		*/


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

		
	};
}
