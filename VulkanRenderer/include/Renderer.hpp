#pragma once
#include "common.hpp"

#include "ShaderSet.hpp"
#include "ResourceManager.hpp"
#include "Image.hpp"

#include <RenderWindow.hpp>

#include "vulkan_base.hpp"

#include "functions.hpp"

#include <taskflow/taskflow.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <functional>

namespace NAME_SPACE {
	struct SwapChain {
		VkSwapchainKHR swapChain = VK_NULL_HANDLE;
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
		BufferPtr srcBuffer;
		TexturePtr srcImage;
		BufferPtr dstBuffer;
		TexturePtr dstImage;
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
		static Renderer* m_myInstance;
	protected:
		RenderWindow* m_window;

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

		SwapChain m_swapChain;
		VkSurfaceKHR m_surface;

		ResourceManager* m_pResourceManager;

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

		void setupDebug();

		void createInstance();
		void getPhysicalDevice();
		void createDevice();

		void createSurface(GLFWwindow* window);

		void createSwapChain();

		void createCommandBuffers();

		void createSyncronisationObjects();

		VkFramebuffer createFramebuffer(VkExtent2D extent, VkRenderPass renderPass, const std::vector<VkImageView>& imageViews);

		uint32_t createGraphicsPipeline(uint32_t renderPass, uint32_t subpassIndex,
			const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,
			const std::vector<VkPushConstantRange>& pushConstantRanges,
			const std::vector<VkPipelineShaderStageCreateInfo> shaderStages,
			VkPipelineVertexInputStateCreateInfo vertexInput);

		uint32_t createComputePipeline(const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,
			const std::vector<VkPushConstantRange>& pushConstantRanges,
			VkPipelineShaderStageCreateInfo shaderStage);

		Renderer(RenderWindow* window);
	public:

		virtual ~Renderer();

		static void init(RenderWindow* window);
		static Renderer* get();
		static void cleanup();

		uint32_t getNextSwapchainImage();

		bool beginFrame();
		void endFrame();

		// create renderpasses, framebuffers and pipelines
		
		uint32_t createRenderPass(const std::vector<VkAttachmentDescription>& attachments,
			const std::vector<VkSubpassDescription>& subpasses,
			const std::vector<VkSubpassDependency>& dependencies);

		VkAttachmentDescription getSwapchainAttachment() const;

		uint32_t createFramebuffer(uint32_t renderPass, const std::vector<TexturePtr>& additionalAttachments);
		
		uint32_t createPipeline(const ShaderSet& shaderSet, uint32_t renderPass = 0, uint32_t subpassIndex = 0);

		//Resource creation
		TexturePtr createDepthImage(VkExtent2D extent);

		TexturePtr createTexture2D(const Image& image);
		TexturePtr createTexture2D(VkExtent2D extent, unsigned char* pixels, int channels = 4);

		SamplerPtr createSampler(VkFilter minMagFilter, float maxAnisotropy = 16.0f, float minLod = 0.0f, float maxLod = 0.0f, float mipLodBias = 0.0f);
		SamplerPtr createSampler(VkSamplerCreateInfo info);


		BufferPtr createVertexBuffer(VkDeviceSize size, void* initialData);
		BufferPtr createIndexBuffer(VkDeviceSize size, void* initialData);

		BufferPtr createUniformBuffer(VkDeviceSize size, void* initialData);
		BufferPtr createStorageBuffer(VkDeviceSize size, void* initialData);


		ShaderPtr createShader(const char* path, VkShaderStageFlagBits stage);
		ShaderSet createShaderSet(const ShaderPtr& vertexShader, const ShaderPtr& fragmentShader);
		ShaderSet createShaderSet(const ShaderPtr& vertexShader, const ShaderPtr& geometryShader, const ShaderPtr& fragmentShader);
		ShaderSet createShaderSet(const ShaderPtr& computeShader);


		CommandBufferPtr createCommandBuffer(bool isCompute = false);
		void recordCommandBuffer(const CommandBufferPtr& commandBuffer, uint32_t framebuffer, uint32_t renderPass, uint32_t subpassIndex, std::function<void(uint32_t frameIndex)> drawCalls, bool isOneTimeRecord = false);
		void recordCommandBuffer(const CommandBufferPtr& commandBuffer, std::function<void(uint32_t frameIndex)> drawCalls, bool isOneTimeRecord = false);
		void executeCommandBuffer(const CommandBufferPtr& commandBuffer, bool isCompute = false);
		void executeCommandBuffers(const std::vector<CommandBufferPtr>& commandBuffers, bool isCompute = false);

		std::shared_ptr<VkFence> submitToComputeQueue(const CommandBufferPtr& commandBuffer);
		void waitForFence(std::shared_ptr<VkFence> fence, uint64_t timeout = UINT64_MAX);

		void updateDescriptorSet(const DescriptorSetPtr& descriptorSet, uint32_t binding, const BufferPtr& buffer, const TexturePtr& image, const SamplerPtr& sampler, bool isOneTimeUpdate);

		// Draw commands
		void beginRenderPass(uint32_t renderPass, uint32_t framebuffer, VkSubpassContents contents, glm::vec3 clearColor = glm::vec3(0.0f));
		void nextSubpass(VkSubpassContents contents);
		void endRenderPass();

		void bindPipeline(uint32_t pipeline, const CommandBufferPtr& commandBuffer = nullptr, uint32_t frameIndex = -1);
		void bindVertexBuffer(const BufferPtr& vertexBuffer, const CommandBufferPtr& commandBuffer = nullptr, uint32_t frameIndex = -1);
		void bindVertexBuffers(const std::vector<BufferPtr>& vertexBuffers, const CommandBufferPtr& commandBuffer = nullptr, uint32_t frameIndex = -1);

		void bindIndexBuffer(const BufferPtr& indexbuffer, const CommandBufferPtr& commandBuffer = nullptr, uint32_t frameIndex = -1);

		void bindDescriptorSet(const DescriptorSetPtr& descriptorSet, uint32_t pipeline, const CommandBufferPtr& commandBuffer = nullptr, uint32_t frameIndex = -1);
		void pushConstants(uint32_t pipeline, VkShaderStageFlags shaderStages, uint32_t offset, uint32_t size, void* data, const CommandBufferPtr& commandBuffer = nullptr, uint32_t frameIndex = -1);

		void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex = 0U, uint32_t firstInstance = 0U, const CommandBufferPtr& commandBuffer = nullptr, uint32_t frameIndex = -1);
		void drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex = 0U, uint32_t firstInstance = 0U, uint32_t vertexOffset = 0U, const CommandBufferPtr& commandBuffer = nullptr, uint32_t frameIndex = -1);

		
	};
}
