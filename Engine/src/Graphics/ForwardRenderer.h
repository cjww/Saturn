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

		vr::DescriptorSetPtr m_pPerFrameDescriptorSet;
		vr::Buffer* m_pPerFrameBuffer;
		vr::Buffer* m_pLightBuffer;

		vr::DescriptorSetPtr m_pInputDescriptorSet;

		uint32_t m_mainFramebuffer;
		vr::Texture* m_pDepthTexture;
		vr::Texture* m_pMainColorTexture;
		vr::Texture* m_pOutputTexture;

		uint32_t m_mainRenderPass;
		uint32_t m_postRenderpass;

		
		uint32_t m_colorPipeline;
		uint32_t m_postProcessPipline;
	

	public:
		ForwardRenderer();
		

		virtual void init(sa::RenderWindow* pWindow, bool setupImGui) override;
		virtual void cleanup() override;

		virtual void beginFrameImGUI() override;

		virtual void draw(Scene* scene) override;

		virtual sa::Texture getOutputTexture() const override;

		virtual sa::Texture createShaderTexture2D(const sa::Image& img) override;
		
	};

}