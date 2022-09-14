#pragma once
#ifndef IMGUI_DISABLE
#include "imgui_impl_vulkan.h"
#include "imgui_impl_glfw.h"
#endif // !IMGUI_DISABLE

#include "Tools/Logger.hpp"
#include "structs.hpp"

#include "CommandPool.hpp"
#include "Resources\ShaderSet.hpp"
#include "Resources/DeviceMemoryManager.hpp"

#include "FormatFlags.hpp"


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
			vk::FrontFace frontFace = vk::FrontFace::eClockwise;
			vk::PolygonMode polygonMode = vk::PolygonMode::eFill;
		} rasterizer;

		struct Multisample {
			vk::Bool32 sampleShadingEnable = false;
			float minSampleShading = 0.0f;
			vk::SampleCountFlagBits sampleCount = vk::SampleCountFlagBits::e1;
		} multisample;
		struct ColorBlend {
			vk::Bool32 enable = true;
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
			vk::Bool32 stencilTestEnable = false;
		} depthStencil;

		std::vector<vk::DynamicState> dynamicStates;

	};

	struct FormatProperties {
		FormatPrecisionFlagBits precision;
		FormatDimensionFlagBits dimension;
		FormatTypeFlagBits type;
		vk::Format format;
	};

	class VulkanCore {
	private:
		vk::ApplicationInfo m_appInfo;
		vk::Instance m_instance;
		vk::Device m_device;
		vk::PhysicalDevice m_physicalDevice;

		std::vector<const char*> m_validationLayers;
		std::vector<const char*> m_instanceExtensions;
		std::vector<const char*> m_deviceExtensions;


		QueueInfo m_queueInfo;
		std::vector<vk::Queue> m_queues;
		
		CommandPool m_mainCommandPool;

		vk::Format m_defaultColorFormat;
		vk::Format m_defaultDepthFormat;

		std::vector<FormatProperties> m_formats;

		DeviceMemoryManager m_memoryManager;

		vk::DescriptorPool m_imGuiDescriptorPool;
		std::unordered_map<vk::ImageView*, VkDescriptorSet> m_imGuiImages;
		vk::Sampler m_imGuiImageSampler;

		void fillFormats();

		uint32_t getQueueFamilyIndex(vk::QueueFlags capabilities, vk::QueueFamilyProperties* prop);
		QueueInfo getQueueInfo(vk::QueueFlags capabilities, uint32_t maxCount);

		void setupDebug();

		void createInstance();
		void findPhysicalDevice();
		void createDevice();

		void createCommandPool();

	public:
		static const unsigned int FRAMES_IN_FLIGHT = 6;

		static bool isDepthFormat(vk::Format format);
		static bool isColorFormat(vk::Format format);

		void init(vk::ApplicationInfo appInfo);
		void cleanup();

#ifndef IMGUI_DISABLE
		void initImGui(GLFWwindow* pWindow, vk::RenderPass renderPass, uint32_t subpass);
		void cleanupImGui();
		VkDescriptorSet getImGuiImageDescriptoSet(vk::ImageView* imageView, vk::ImageLayout layout);
#endif
		
		vk::SurfaceKHR createSurface(GLFWwindow* pWindow);
		vk::SwapchainKHR createSwapchain(vk::SurfaceKHR surface, vk::Format* outFormat, vk::Extent2D* outExtent);
		vk::ImageView createImageView(vk::ImageViewType type, vk::Image image, vk::Format format, vk::ImageAspectFlags aspectMask,
			uint32_t mipLevels, uint32_t baseMipLevel, uint32_t layers, uint32_t baseArrayLevel);
		vk::BufferView createBufferView(vk::Buffer buffer, vk::Format format);

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

		CommandBufferSet allocateCommandBufferSet(vk::CommandBufferLevel level);
		CommandBufferSet allocateCommandBufferSet(vk::CommandBufferLevel level, CommandPool& commandPool);

		DeviceBuffer* createBuffer(vk::BufferUsageFlags usage, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags allocationFlags, size_t size, void* initialData);
		void destroyBuffer(DeviceBuffer* pBuffer);

		DeviceImage* createImage2D(Extent extent, vk::Format format, vk::ImageUsageFlags usage, vk::SampleCountFlagBits sampleCount, uint32_t mipLevels = 1, uint32_t arrayLayers = 1, vk::ImageCreateFlags flags = (vk::ImageCreateFlags)0);
		DeviceImage* createImage3D(Extent3D extent, vk::Format format, vk::ImageUsageFlags usage, vk::SampleCountFlagBits sampleCount, uint32_t mipLevels = 1, uint32_t arrayLayers = 1, vk::ImageCreateFlags flags = (vk::ImageCreateFlags)0);
		void destroyImage(DeviceImage* pImage);

		void transferImageLayout(vk::CommandBuffer commandBuffer,
			vk::ImageLayout oldLayout,
			vk::ImageLayout newLayout,
			vk::AccessFlags srcAccessMask,
			vk::AccessFlags dstAccessMask,
			vk::Image image,
			vk::ImageAspectFlags imageAspect,
			uint32_t mipLevels,
			uint32_t layers,
			vk::PipelineStageFlags srcStage,
			vk::PipelineStageFlags dstStage);

		void transferBufferToColorImage(vk::CommandBuffer commandBuffer,
			vk::Buffer buffer,
			vk::Image image,
			uint32_t mipLevels, 
			uint32_t layers,
			vk::Extent3D copyExtent,
			vk::ImageLayout oldLayout,
			vk::ImageLayout newLayout,
			vk::AccessFlags dstAccessMask,
			vk::PipelineStageFlags dstStage);

		void generateMipmaps(vk::CommandBuffer commandBuffer, vk::Image image, vk::Extent3D extent, uint32_t mipLevels);

		vk::Sampler createSampler(const vk::SamplerCreateInfo& info);

		// Get functions
		uint32_t getQueueFamily() const;
		uint32_t getQueueCount() const;

		vk::Instance getInstance() const;
		vk::PhysicalDevice getPhysicalDevice() const;
		vk::Device getDevice() const;


		vk::Format getDefaultColorFormat() const;
		vk::Format getDefaultDepthFormat() const;

		vk::Format getSupportedDepthFormat();
		vk::Format getFormat(const std::vector<vk::Format>& candidates, vk::FormatFeatureFlags features, vk::ImageTiling tilling);
		vk::Format getFormat(FormatPrecisionFlags precision, FormatDimensionFlags dimensions, FormatTypeFlags type, vk::FormatFeatureFlags features, vk::ImageTiling tilling);

		vk::SampleCountFlags getSupportedColorSampleCounts() const;
		vk::SampleCountFlags getSupportedDepthSampleCounts() const;

		DeviceMemoryStats getGPUMemoryUsage() const;

		// used by memory manager to check memory bugets 
		void setMemoryManagerFrameIndex(uint32_t frameIndex);

	};


}
