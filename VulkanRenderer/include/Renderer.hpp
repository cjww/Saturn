#pragma once
#include "ApiBuildOptions.h"

#include "RenderProgramFactory.hpp"

#include "RenderContext.hpp"
#include "Resources/Buffer.hpp"
#include "Resources/Texture.hpp"
#include "Resources/DynamicTexture.hpp"
#include "Image.hpp"
#include "FormatFlags.hpp"
#include "PipelineSettings.hpp"
#include "DeviceMemoryStats.hpp"

#include "ShaderSet.hpp"

#include "Window.hpp"

#include "imgui.h"
#include "imgui_stdlib.h"

#include <mutex>

#include "Resources/DynamicBuffer.hpp"

namespace sa {

	class VulkanCore;
	class CommandBufferSet;

	// Uses Vulkan values
	enum class FilterMode {
		NEAREST = 0,
		LINEAR = 1,
		CUBIC = 1000015000
	};

	enum class SamplerAddressMode {
		REPEAT = 0,
		MIRRORED_REPEAT = 1,
		CLAMP_TO_EDGE = 2,
		CLAMP_TO_BORDER = 3,
		MIRROR_CLAMP_TO_EDGE = 4,
		MAX_ENUM = 0x7FFFFFFF
	};

	enum class SamplerMipmapMode {
		NEAREST = 0,
		LINEAR = 1,
		MAX_ENUM = 0x7FFFFFFF
	};

	enum class CompareOp {
		NEVER = 0,
		LESS = 1,
		EQUAL = 2,
		LESS_OR_EQUAL = 3,
		GREATER = 4,
		NOT_EQUAL = 5,
		GREATER_OR_EQUAL = 6,
		ALWAYS = 7,
		MAX_ENUM = 0x7FFFFFFF
	};

	enum class BorderColor {
		FLOAT_TRANSPARENT_BLACK = 0,
		INT_TRANSPARENT_BLACK = 1,
		FLOAT_OPAQUE_BLACK = 2,
		INT_OPAQUE_BLACK = 3,
		FLOAT_OPAQUE_WHITE = 4,
		INT_OPAQUE_WHITE = 5,
		FLOAT_CUSTOM_EXT = 1000287003,
		INT_CUSTOM_EXT = 1000287004,
		MAX_ENUM = 0x7FFFFFFF
	};

	struct SamplerInfo {
		FilterMode				magFilter = FilterMode::LINEAR;
		FilterMode				minFilter = FilterMode::LINEAR;
		SamplerMipmapMode		mipmapMode = SamplerMipmapMode::NEAREST;
		SamplerAddressMode		addressModeU = SamplerAddressMode::REPEAT;
		SamplerAddressMode		addressModeV = SamplerAddressMode::REPEAT;
		SamplerAddressMode		addressModeW = SamplerAddressMode::REPEAT;
		float                   mipLodBias = 0.f;
		bool					anisotropyEnable = false;
		float                   maxAnisotropy = 0.f;
		bool					compareEnable = false;
		CompareOp				compareOp = CompareOp::NEVER;
		float                   minLod = 0.f;
		float                   maxLod = 9.f;
		BorderColor				borderColor = BorderColor::FLOAT_TRANSPARENT_BLACK;
		bool					unnormalizedCoordinates = false;
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
		
		std::list<DataTransfer> m_transferQueue;
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

		VulkanCore* getCore() const;

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

		ResourceID createSwapchainFramebuffer(ResourceID renderProgram, ResourceID swapchain, uint32_t layers = 1ui32);

		void destroyFramebuffer(ResourceID framebuffer);

		Texture getFramebufferTexture(ResourceID framebuffer, uint32_t attachmentIndex) const;
		DynamicTexture getFramebufferDynamicTexture(ResourceID framebuffer, uint32_t attachmentIndex) const;
		DynamicTexture* getFramebufferDynamicTexturePtr(ResourceID framebuffer, uint32_t attachmentIndex) const;

		size_t getFramebufferTextureCount(ResourceID framebuffer) const;
		Extent getFramebufferExtent(ResourceID framebuffer) const;
		void swapFramebuffer(ResourceID framebuffer);

		ResourceID createComputePipeline(const Shader& shader, const PipelineLayout& layout);
		ResourceID createGraphicsPipeline(PipelineLayout& layout, Shader* pShaders, uint32_t shaderCount, ResourceID renderProgram, uint32_t subpassIndex, Extent extent, PipelineSettings settings = {});
		void destroyPipeline(ResourceID pipeline);

		void updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const Buffer& buffer);
		void updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, DynamicBuffer& buffer);
		void updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const Texture& texture, ResourceID sampler);
		void updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const Texture& texture);
		void updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const DynamicTexture& texture, ResourceID sampler);
		void updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const DynamicTexture& texture);
		void updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const std::vector<Texture>& textures, uint32_t firstElement);
		void updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const std::vector<Texture>& textures, ResourceID sampler, uint32_t firstElement);
		void updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const Texture* textures, uint32_t textureCount, ResourceID sampler, uint32_t firstElement);
		void updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const Texture* textures, uint32_t textureCount, uint32_t firstElement);

		void updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, ResourceID sampler);

		void freeDescriptorSet(ResourceID descriptorSet);

		Buffer createBuffer(BufferType type, size_t size = 0ui64, void* initialData = nullptr);
		DynamicBuffer createDynamicBuffer(BufferType type, size_t size = 0ui64, void* initialData = nullptr);

		DeviceMemoryStats getGPUMemoryUsage() const;

		DataTransfer* queueTransfer(const DataTransfer& transfer);
		bool cancelTransfer(DataTransfer* pTransfer);

		ResourceID createSampler(FilterMode filterMode = FilterMode::NEAREST);
		ResourceID createSampler(const SamplerInfo& samplerInfo);
		
		RenderContext beginFrame(ResourceID swapchain);
		void endFrame(ResourceID swapchain);

		ResourceID createContextPool();

		DirectContext createDirectContext(ResourceID contextPool = NULL_RESOURCE);
		SubContext createSubContext(ResourceID framebuffer, ResourceID renderProgram, uint32_t subpassIndex, ResourceID contextPool = NULL_RESOURCE);
		SubContext createSubContext(ResourceID contextPool = NULL_RESOURCE);
		
		Format selectFormat(const std::vector<Format>& formatCandidates, TextureTypeFlags textureType) const;
		Format getAttachmentFormat(ResourceID renderProgram, uint32_t attachmentIndex) const;

	};


	
}

namespace ImGui {
	void Image(sa::Texture texture, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1), const ImVec4& tint_col = ImVec4(1, 1, 1, 1), const ImVec4& border_col = ImVec4(0, 0, 0, 0));
	bool ImageButton(sa::Texture texture, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1), int frame_padding = -1, const ImVec4& bg_col = ImVec4(0, 0, 0, 0), const ImVec4& tint_col = ImVec4(1, 1, 1, 1));

}