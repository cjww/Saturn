#include "ForwardRenderer.h"

ForwardRenderer::ForwardRenderer() {
	
}

void ForwardRenderer::init(RenderWindow* pWindow, bool setupImGui) {
	vr::Renderer::init(pWindow);
	m_renderer = vr::Renderer::get();
	m_useImGui = setupImGui;

	vr::ShaderPtr vertexShader = m_renderer->createShader("../Engine/shaders/TextureVertexShader.spv", VK_SHADER_STAGE_VERTEX_BIT);
	vr::ShaderPtr fragmentShader = m_renderer->createShader("../Engine/shaders/TextureFragmentShader.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
	m_pShaderSet = m_renderer->createShaderSet(vertexShader, fragmentShader);


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
	m_frameBuffer = m_renderer->createSwapchainFramebuffer(m_renderPass, { m_pDepthTexture });

	m_pipeline = m_renderer->createPipeline(m_pShaderSet, m_renderPass, 0);
	
	if (m_useImGui) {
		m_renderer->initImGUI(m_renderPass);
	}

	ECSCoordinator::get()->registerComponent<Model>();
	ECSCoordinator::get()->registerComponent<Transform>();
	
	m_pDescriptorManager = new vk::DescriptorManager(m_pShaderSet.get());


	ComponentMask mask;
	mask.set(ECSCoordinator::get()->getComponentType<Transform>());
	m_pDescriptorCreationSystem = ECSCoordinator::get()->registerSystem<vk::DescriptorCreationSystem>(mask, m_pDescriptorManager);

	mask.reset();
	mask.set(ECSCoordinator::get()->getComponentType<Model>());
	mask.set(ECSCoordinator::get()->getComponentType<Transform>());
	m_pMeshRenderSystem = ECSCoordinator::get()->registerSystem<vk::MeshRenderSystem>(mask, m_pDescriptorManager);

	

	PerFrameBuffer perFrame = {};
	glm::mat4 proj = glm::perspective(glm::radians(60.f), 1000.f / 600.f, 0.001f, 100.0f);
	glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 1), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	perFrame.projViewMatrix[0] = proj * view;
	m_pPerFrameBuffer = m_renderer->createUniformBuffer(sizeof(PerFrameBuffer), &perFrame);
	
	m_pPerFrameDescriptorSet = m_pShaderSet->getDescriptorSet(SET_PER_FRAME);
	m_renderer->updateDescriptorSet(m_pPerFrameDescriptorSet, 0, m_pPerFrameBuffer, nullptr, nullptr, true);
}

void ForwardRenderer::cleanup() {
	m_pShaderSet = nullptr;
	m_pPerFrameDescriptorSet = nullptr;
	if (m_useImGui) {
		m_renderer->cleanupImGUI();
	}

	delete m_pDescriptorManager;
	vr::Renderer::cleanup();
}

void ForwardRenderer::beginFrameImGUI() {
	if (m_useImGui && !m_renderer->getWindow()->wasResized()) {
		m_renderer->newFrameImGUI();
	}
}

bool ForwardRenderer::beginFrame() {
	bool begin = m_renderer->beginFrame();
	if (!begin)
		return false;
	m_renderer->beginRenderPass(m_renderPass, m_frameBuffer, VK_SUBPASS_CONTENTS_INLINE);
	m_renderer->bindPipeline(m_pipeline);
	m_renderer->bindDescriptorSet(m_pPerFrameDescriptorSet, m_pipeline);

	return true;
}

void ForwardRenderer::endFrame() {
	
	m_renderer->endRenderPass();
	if (m_useImGui) {
		m_renderer->endFrameImGUI();
	}
	m_renderer->endFrame();
}


void ForwardRenderer::draw() {
	if (beginFrame()) {
		
		m_pMeshRenderSystem->draw(m_pipeline);

		endFrame();
	}
}
