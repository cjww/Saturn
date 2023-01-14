#pragma once

#include "RenderProgramFactory.hpp"

#include "RenderContext.hpp"
#include "Resources/Buffer.hpp"
#include "Resources/Texture.hpp"
#include "Resources/DynamicTexture.hpp"
#include "Image.hpp"
#include "FormatFlags.hpp"
#include "PipelineSettings.hpp"
#include "DeviceMemoryStats.hpp"

#include "Window.hpp"

#include "imgui.h"
#include "imgui_stdlib.h"

#include <mutex>

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
	private:
		friend class Texture;
		friend class DynamicTexture;

		std::unique_ptr<VulkanCore> m_pCore;
		
		std::queue<DataTransfer> m_transferQueue;
		std::mutex m_transferMutex;

		const bool c_useVaildationLayers =
#if SA_RENDER_VALIDATION_ENABLE
		true;
#else
		false;
#endif


		Renderer();
	public:
		static Renderer& get();
		virtual ~Renderer();

#ifndef IMGUI_DISABLE
		void initImGui(const Window& window, ResourceID renderProgram, uint32_t subpass);
		void newImGuiFrame();
		void cleanupImGui();
		void imGuiImage(sa::Texture texture, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, const ImVec4& border_col);
		bool imGuiImageButton(sa::Texture texture, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, int frame_padding, const ImVec4& bg_col, const ImVec4& tint_col);

		ImTextureID getImGuiTexture(const sa::Texture* texture);

#endif // !IMGUI_DISABLE

		ResourceID createSwapchain(GLFWwindow* pWindow);
		ResourceID recreateSwapchain(GLFWwindow* pWindow, ResourceID oldSwapchain);
		void destroySwapchain(ResourceID swapchain);

		uint32_t getSwapchainImageCount(ResourceID swapchain);

		void waitForFrame(ResourceID swapchains);

		RenderProgramFactory createRenderProgram();
		void destroyRenderProgram(ResourceID renderProgram);
		void setClearColor(ResourceID renderProgram, Color color, uint32_t attachmentIndex);
		void setClearColor(ResourceID renderProgram, Color color);

		ResourceID createFramebuffer(ResourceID renderProgram, const std::vector<DynamicTexture>& attachmentTextures, uint32_t layers = 1ui32);
		ResourceID createSwapchainFramebuffer(ResourceID renderProgram, ResourceID swapchain, const std::vector<DynamicTexture>& additionalAttachmentTextures, uint32_t layers = 1ui32);

		ResourceID createFramebuffer(ResourceID renderProgram, const std::vector<Texture>& attachmentTextures, uint32_t layers = 1ui32);
		ResourceID createSwapchainFramebuffer(ResourceID renderProgram, ResourceID swapchain, const std::vector<Texture>& additionalAttachmentTextures, uint32_t layers = 1ui32);

		void destroyFramebuffer(ResourceID framebuffer);

		Texture getFramebufferTexture(ResourceID framebuffer, uint32_t index) const;
		size_t getFramebufferTextureCount(ResourceID framebuffer) const;

		ResourceID createGraphicsPipeline(ResourceID renderProgram, uint32_t subpassIndex, Extent extent, const std::string& vertexShader, PipelineSettings settings = {});
		ResourceID createGraphicsPipeline(ResourceID renderProgram, uint32_t subpassIndex, Extent extent, const std::string& vertexShader, const std::string& fragmentShader, PipelineSettings settings = {});
		ResourceID createGraphicsPipeline(ResourceID renderProgram, uint32_t subpassIndex, Extent extent, const std::string& vertexShader, const std::string& geometryShader, const std::string& fragmentShader, PipelineSettings settings = {});

		ResourceID createComputePipeline(const std::string& computeShader);
		void destroyPipeline(ResourceID pipeline);

		ResourceID allocateDescriptorSet(ResourceID pipeline, uint32_t setIndex);
		void updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const Buffer& buffer);
		void updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, DynamicBuffer& buffer);
		void updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const Texture& texture, ResourceID sampler);
		void updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const Texture& texture);
		void updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const DynamicTexture& texture, ResourceID sampler);
		void updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const DynamicTexture& texture);
		void updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const std::vector<Texture>& textures, uint32_t firstElement = 0);
		void updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, ResourceID sampler);

		void freeDescriptorSet(ResourceID descriptorSet);

		Buffer createBuffer(BufferType type, size_t size = 0ui64, void* initialData = nullptr);
		DynamicBuffer createDynamicBuffer(BufferType type, size_t size = 0ui64, void* initialData = nullptr);

		DeviceMemoryStats getGPUMemoryUsage() const;

		void queueTransfer(const DataTransfer& transfer);

		ResourceID createSampler(FilterMode filterMode = FilterMode::NEAREST);

		RenderContext beginFrame(ResourceID swapchain);
		void endFrame(ResourceID swapchain);

		ResourceID createContextPool();

		DirectContext createDirectContext(ResourceID contextPool = NULL_RESOURCE);
		SubContext createSubContext(ResourceID framebuffer, ResourceID renderProgram, uint32_t subpassIndex, ResourceID contextPool = NULL_RESOURCE);
		SubContext createSubContext(ResourceID contextPool = NULL_RESOURCE);
	};


	
}

namespace ImGui {
	void Image(sa::Texture texture, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1), const ImVec4& tint_col = ImVec4(1, 1, 1, 1), const ImVec4& border_col = ImVec4(0, 0, 0, 0));
	bool ImageButton(sa::Texture texture, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1), int frame_padding = -1, const ImVec4& bg_col = ImVec4(0, 0, 0, 0), const ImVec4& tint_col = ImVec4(1, 1, 1, 1));

}