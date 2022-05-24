#pragma once


#include "Tools/Logger.hpp"

#include "CommandPool.hpp"
#include "Resources/FramebufferSet.hpp"

#include "Resources\ShaderSet.hpp"

namespace sa {

	struct QueueInfo {
		uint32_t family = 0;
		std::vector<float> priorities;
		uint32_t queueCount;
	};

	struct PipelineConfig {
		struct InputAssembly {
			vk::PrimitiveTopology topology = vk::PrimitiveTopology::eTriangleList;
		} input;

		struct Rasterizer {
			vk::CullModeFlags cullMode = vk::CullModeFlagBits::eBack;
			vk::FrontFace frontFace = vk::FrontFace::eCounterClockwise;
			vk::PolygonMode polygonMode = vk::PolygonMode::eFill;
		} rasterizer;

		struct Multisample {
			vk::Bool32 enable = false;
			vk::SampleCountFlagBits sampleCount = vk::SampleCountFlagBits::e1;
		} multisample;
		struct ColorBlend {
			vk::Bool32 enable = false;
			vk::BlendOp colorBlendOp = vk::BlendOp::eAdd;
			vk::BlendOp alphaBlendOp = vk::BlendOp::eAdd;
			vk::BlendFactor srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
			vk::BlendFactor dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
			vk::BlendFactor srcAlphaBlendFactor = vk::BlendFactor::eOne;
			vk::BlendFactor dstAlphaBlendFactor = vk::BlendFactor::eZero;
		};
		std::vector<ColorBlend> colorBlends;

		struct DepthStencilState {
			vk::Bool32 depthBoundsTestEnable = false;
			vk::CompareOp depthCompareOp = vk::CompareOp::eLessOrEqual;
			vk::Bool32 depthTestEnable = true;
			vk::Bool32 depthWriteEnable = true;
			vk::Bool32 stencilTestEnable = VK_FALSfalse;
		} depthStencil;

		std::vector<vk::DynamicState> dynamicStates;

	};

	class VulkanCore {
	private:

		vk::Instance m_instance;
		vk::Device m_device;
		vk::PhysicalDevice m_physicalDevice;

		std::vector<const char*> m_validationLayers;
		std::vector<const char*> m_instanceExtensions;
		std::vector<const char*> m_deviceExtensions;
		

		QueueInfo m_graphicsQueueInfo;
		QueueInfo m_computeQueueInfo;
		vk::Queue m_graphicsQueue;
		vk::Queue m_computeQueue;

		CommandPool m_graphicsCommandPool;
		CommandPool m_computeCommandPool;

		vk::Format m_defaultColorFormat;
		vk::Format m_defaultDepthFormat;


		uint32_t getQueueFamilyIndex(vk::QueueFlags capabilities, vk::QueueFamilyProperties* prop);
		QueueInfo getQueueInfo(vk::QueueFlags capabilities, uint32_t maxCount);

		void setupDebug();

		void createInstance();
		void findPhysicalDevice();
		void createDevice();

		void createCommandPools();

	public:

		void init();
		void cleanup();

		vk::SurfaceKHR createSurface(GLFWwindow* pWindow);
		vk::SwapchainKHR createSwapchain(vk::SurfaceKHR surface, uint32_t queueFamily, vk::Format* outFormat);
		vk::ImageView createImageView(vk::ImageViewType type, vk::Image image, vk::Format format, vk::ImageAspectFlags aspectMask, uint32_t baseMipLevel, uint32_t baseArrayLevel);
		
		vk::RenderPass createRenderPass(std::vector<vk::AttachmentDescription> attachments, std::vector<vk::SubpassDescription> subpasses, std::vector<vk::SubpassDependency> dependencies);
		vk::Framebuffer createFrameBuffer(vk::RenderPass renderPass, std::vector<vk::ImageView> attachments, uint32_t width, uint32_t height, uint32_t layers);

		vk::Pipeline createGraphicsPipeline(
			vk::PipelineLayout layout,
			vk::RenderPass renderPass,
			uint32_t subpassIndex,
			vk::Extent2D extent,
			std::vector<vk::PipelineShaderStageCreateInfo> shaderStages,
			vk::PipelineVertexInputStateCreateInfo vertexInput,
			vk::PipelineCache cache,
			PipelineConfig config);

		CommandBufferSet allocateGraphicsCommandBufferSet(uint32_t count, vk::CommandBufferLevel level);
		CommandBufferSet allocateComputeCommandBufferSet(uint32_t count, vk::CommandBufferLevel level);
		FramebufferSet createFrameBufferSet(
			vk::RenderPass renderPass, 
			std::vector<std::vector<vk::ImageView>> attachments, 
			uint32_t width, 
			uint32_t height, 
			uint32_t layers);


		uint32_t getGraphicsQueueFamily() const;
		vk::Queue getGraphicsQueue() const;
		uint32_t getComputeQueueFamily() const;
		vk::Queue getComputeQueue() const;

		vk::Instance getInstance() const;
		vk::PhysicalDevice getPhysicalDevice() const;
		vk::Device getDevice() const;


		vk::Format getDefaultColorFormat() const;
		vk::Format getDefaultDepthFormat() const;


	};
}
