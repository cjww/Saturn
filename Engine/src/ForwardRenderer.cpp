#include "ForwardRenderer.h"

ForwardRenderer::ForwardRenderer() {
	
}

void ForwardRenderer::init(RenderWindow* pWindow) {
	vr::Renderer::init(pWindow);
	m_renderer = vr::Renderer::get();

	vr::ShaderPtr vertexShader = m_renderer->createShader("../Engine/shaders/TextureVertexShader.spv", VK_SHADER_STAGE_VERTEX_BIT);
	vr::ShaderPtr fragmentShader = m_renderer->createShader("../Engine/shaders/TextureFragmentShader.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
	vr::ShaderSet shaderSet = m_renderer->createShaderSet(vertexShader, fragmentShader);


	glm::ivec2 extent = m_renderer->getWindow()->getCurrentExtent();

	m_pDepthTexture = m_renderer->createDepthImage({ (uint32_t)extent.x, (uint32_t)extent.y });

	std::vector<VkAttachmentDescription> attachments(2);
	attachments[0] = m_renderer->getSwapchainAttachment();
	attachments[1] = vr::getDepthAttachment(m_pDepthTexture->format, m_pDepthTexture->sampleCount);

	std::vector<VkAttachmentReference> refrences(2);
	refrences[0].attachment = 0;
	refrences[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	refrences[1].attachment = 1;
	refrences[1].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	std::vector<VkSubpassDescription> subpasses(1);
	subpasses[0].flags = 0;
	subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpasses[0].inputAttachmentCount = 0;
	subpasses[0].preserveAttachmentCount = 0;

	subpasses[0].colorAttachmentCount = 1;
	subpasses[0].pColorAttachments = &refrences[0];
	subpasses[0].pDepthStencilAttachment = &refrences[1];

	m_renderPass = m_renderer->createRenderPass(attachments, subpasses, {});
	m_frameBuffer = m_renderer->createFramebuffer(m_renderPass, { m_pDepthTexture });

	m_pipeline = m_renderer->createPipeline(shaderSet, m_renderPass, 0);


	ECSCoordinator::get()->registerComponent<Model>();
	ECSCoordinator::get()->registerComponent<Transform>();

	ComponentMask mask;
	mask.set(ECSCoordinator::get()->getComponentType<Model>());
	mask.set(ECSCoordinator::get()->getComponentType<Transform>());
	m_pMeshRenderSystem = ECSCoordinator::get()->registerSystem<vk::MeshRenderSystem>(mask);
}

void ForwardRenderer::cleanup() {
	m_pDepthTexture = nullptr;
	vr::Renderer::cleanup();
}

bool ForwardRenderer::beginFrame() {
	bool begin = m_renderer->beginFrame();
	if (!begin)
		return false;

	m_renderer->beginRenderPass(m_renderPass, m_frameBuffer, VK_SUBPASS_CONTENTS_INLINE);
	m_renderer->bindPipeline(m_pipeline);


	return true;
}

void ForwardRenderer::endFrame() {
	m_renderer->endRenderPass();
	m_renderer->endFrame();
}


void ForwardRenderer::draw() {
	if (beginFrame()) {

		m_pMeshRenderSystem->draw();

		endFrame();
	}
}
