#pragma once

#include "RenderProgramFactory.hpp"

#include "RenderContext.hpp"
#include "Resources/Buffer.hpp"
#include "Resources/Texture.hpp"
#include "Image.hpp"
#include "FormatFlags.hpp"

#include "Window.hpp"

#include "imgui.h"


namespace sa {

	class VulkanCore;
	class CommandBufferSet;

	// Uses Vulkan values
	enum class FilterMode {
		NEAREST = 0,
		LINEAR = 1,
		CUBIC = 1000015000
	};

	struct DataTransfer {
		enum Type {
			BUFFER_TO_IMAGE,
			BUFFER_TO_BUFFER,
			IMAGE_TO_BUFFER,
			IMAGE_TO_IMAGE
		} type;
		DeviceBuffer* srcBuffer = nullptr;
		DeviceImage* srcImage = nullptr;
		DeviceBuffer* dstBuffer = nullptr;
		DeviceImage* dstImage = nullptr;
	};

	class Renderer {
	protected:
		
		std::unique_ptr<VulkanCore> m_pCore;
		std::queue<DataTransfer> m_transferQueue;

		Renderer();
	public:
		static Renderer& get();
		virtual ~Renderer();
#ifndef IMGUI_DISABLE
		void initImGui(const Window& window, ResourceID renderProgram, uint32_t subpass);
		void newImGuiFrame();

		void imGuiImage(sa::Texture texture, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, const ImVec4& border_col);

#endif // !IMGUI_DISABLE
		
		/*
#ifdef SA_INCLUDE_IMGUI
		std::unordered_map<Texture, ImTextureID> m_imGuiImages;

		void createImGUIDescriptorPool();
		ImGui_ImplVulkan_InitInfo getImGUIInitInfo() const;
#endif
		*/

		ResourceID createSwapchain(GLFWwindow* pWindow);
		ResourceID recreateSwapchain(GLFWwindow* pWindow, ResourceID oldSwapchain);
		void destroySwapchain(ResourceID swapchain);

		uint32_t getSwapchainImageCount(ResourceID swapchain);

		RenderProgramFactory createRenderProgram();
		void setClearColor(ResourceID renderProgram, Color color, uint32_t attachmentIndex);
		void setClearColor(ResourceID renderProgram, Color color);

		ResourceID createFramebuffer(ResourceID renderProgram, const std::vector<Texture>& attachmentTextures, uint32_t layers = 1ui32);
		ResourceID createSwapchainFramebuffer(ResourceID renderProgram, ResourceID swapchain, const std::vector<Texture>& additionalAttachmentTextures, uint32_t layers = 1ui32);
		void destroyFramebuffer(ResourceID framebuffer);

		ResourceID createGraphicsPipeline(ResourceID renderProgram, uint32_t subpassIndex, Extent extent, const std::string& vertexShader, const std::string& fragmentShader);
		ResourceID createGraphicsPipeline(ResourceID renderProgram, uint32_t subpassIndex, Extent extent, const std::string& vertexShader, const std::string& geometryShader, const std::string& fragmentShader);
		ResourceID createComputePipeline(const std::string& computeShader);
		void destroyPipeline(ResourceID pipeline);

		ResourceID allocateDescriptorSet(ResourceID pipeline, uint32_t setIndex);
		void updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const Buffer& buffer);
		void updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const Texture2D& texture, ResourceID sampler);
		void updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const Texture2D& texture);

		void freeDescriptorSet(ResourceID descriptorSet);

		Buffer createBuffer(BufferType type, size_t size = 0ui64, void* initialData = nullptr);
		Texture2D createTexture2D(TextureTypeFlags type, Extent extent, uint32_t sampleCount = 1);
		Texture2D createTexture2D(TextureTypeFlags type, Extent extent, FormatPrecisionFlags formatPrecision, FormatDimensionFlags formatDimensions, FormatTypeFlags formatType, uint32_t sampleCount = 1);
		Texture2D createTexture2D(TextureTypeFlags type, Extent extent, ResourceID swapchain, uint32_t sampleCount);
		Texture2D createTexture2D(const Image& image);

		void queueTransfer(const DataTransfer& transfer);

		ResourceID createSampler(FilterMode filterMode = FilterMode::NEAREST);

		RenderContext beginFrame(ResourceID swapchain);
		void endFrame(ResourceID swapchain);

		ResourceID createContextPool();

		DirectContext createDirectContext(ResourceID contextPool = NULL_RESOURCE);
		SubContext createSubContext(ResourceID framebuffer, ResourceID renderProgram, uint32_t subpassIndex, ResourceID contextPool = NULL_RESOURCE);

		/*
		void initImGUI(GLFWwindow* window, uint32_t renderpass, uint32_t subpass);
		void newFrameImGUI();
		void endFrameImGUI();
		void cleanupImGUI();

		ImTextureID getImTextureID(Texture* texture, const SamplerPtr& sampler);

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


		void waitDeviceIdle() const;

		void updateDescriptorSet(const DescriptorSetPtr& descriptorSet, uint32_t binding, const Buffer* buffer, const Texture* image, const SamplerPtr& sampler, bool isOneTimeUpdate);

		// Draw commands
		
		void bindViewports(const std::vector<VkViewport>& viewports, const CommandBufferPtr& commandBuffer = nullptr, uint32_t frameIndex = -1);
		void bindViewport(const VkViewport& viewport, const CommandBufferPtr& commandBuffer = nullptr, uint32_t frameIndex = -1);
		*/

	};


	
}

namespace ImGui {
	void Image(sa::Texture texture, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1), const ImVec4& tint_col = ImVec4(1, 1, 1, 1), const ImVec4& border_col = ImVec4(0, 0, 0, 0));
}