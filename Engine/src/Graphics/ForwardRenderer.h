#pragma once
#include "IRenderTechnique.h"

//#include <Rendering/Vulkan/Renderer.hpp>

#include "structs.h"
#include "ECS\Components.h"

#include "Graphics\RenderWindow.h"
#include "Graphics\Image.h"

namespace vr {
	class Renderer;
	
	class ShaderSet;
	typedef std::shared_ptr<ShaderSet> ShaderSetPtr;

	class DescriptorSet;
	typedef std::shared_ptr<DescriptorSet> DescriptorSetPtr;

	struct Buffer;
	struct Texture;
	struct Image;
}

namespace sa {
	class ForwardRenderer : public IRenderTechnique {
	private:
		vr::Renderer* m_renderer;

		vr::ShaderSetPtr m_pColorShaders;
		vr::ShaderSetPtr m_pPostProcessShaders;

		vr::ShaderSetPtr m_pBlurComputeShader;
		vr::DescriptorSetPtr m_pBlurDescriptorSet;
		
		vr::DescriptorSetPtr m_pPerFrameDescriptorSet;
		vr::Buffer* m_pPerFrameBuffer;
		vr::Buffer* m_pLightBuffer;

		vr::DescriptorSetPtr m_pInputDescriptorSet;

		uint32_t m_mainFramebuffer;
		uint32_t m_postFramebuffer;
		uint32_t m_imguiFramebuffer;

		vr::Texture* m_pDepthTexture;
		vr::Texture* m_pMainColorTexture;

		vr::Texture* m_pBrightnessTexture;
		vr::Texture* m_pBlurredBrightnessTexture;
		
		vr::Texture* m_pOutputTexture;
		
		vr::SamplerPtr m_sampler;
		
		uint32_t m_mainRenderPass;
		uint32_t m_postRenderpass;
		uint32_t m_imguiRenderpass;
		
		uint32_t m_colorPipeline;
		uint32_t m_postProcessPipline;

		uint32_t m_blurPipeline;

		vr::CommandBufferPtr m_blurCommandBuffer;

		void createTextures(VkExtent2D extent);
		void createRenderPasses();
		void createFramebuffers(VkExtent2D extent);
		void createPipelines(VkExtent2D extent);

	public:
		ForwardRenderer();
		
		void swapchainResizedCallback(uint32_t width, uint32_t height);


		virtual void init(sa::RenderWindow* pWindow, bool setupImGui) override;
		virtual void cleanup() override;

		virtual void beginFrameImGUI() override;

		virtual void draw(Scene* scene) override;

		virtual sa::Texture getOutputTexture() const override;

		virtual sa::Texture createShaderTexture2D(const sa::Image& img) override;
		
	};

}