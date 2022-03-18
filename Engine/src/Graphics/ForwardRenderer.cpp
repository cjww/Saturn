#include "pch.h"
#include "ForwardRenderer.h"

#include "Graphics\Vulkan\Renderer.hpp"

namespace sa {
	
	void ForwardRenderer::createTextures(VkExtent2D extent) {
		m_pMainColorTexture = m_renderer->createColorAttachmentTexture(
			extent,
			VK_FORMAT_R8G8B8A8_UNORM,
			1,
			1,
			VK_SAMPLE_COUNT_1_BIT,
			VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

		m_pDepthTexture = m_renderer->createDepthTexture(extent);

		m_pOutputTexture = m_renderer->createColorAttachmentTexture(
			extent,
			VK_FORMAT_R8G8B8A8_UNORM,
			1,
			1,
			VK_SAMPLE_COUNT_1_BIT,
			VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
	}

	void ForwardRenderer::createRenderPasses() {
		std::vector<VkAttachmentDescription> mainAttachments(2);
		mainAttachments[0] = vr::getColorAttachment(m_pMainColorTexture->format, m_pMainColorTexture->sampleCount, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		mainAttachments[1] = vr::getDepthAttachment(m_pDepthTexture->format, m_pDepthTexture->sampleCount);



		std::vector<VkAttachmentReference> mainPassReferences(2);
		mainPassReferences[0].attachment = 0; // m_pMainColorTexture / output
		mainPassReferences[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		mainPassReferences[1].attachment = 1; // m_pDepthTexture
		mainPassReferences[1].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


		VkSubpassDescription mainRenderpass;
		mainRenderpass.flags = 0;
		mainRenderpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		mainRenderpass.inputAttachmentCount = 0;
		mainRenderpass.preserveAttachmentCount = 0;

		mainRenderpass.colorAttachmentCount = 1;
		mainRenderpass.pColorAttachments = &mainPassReferences[0];
		mainRenderpass.pDepthStencilAttachment = &mainPassReferences[1];
		mainRenderpass.pResolveAttachments = nullptr;


		m_mainRenderPass = m_renderer->createRenderPass(mainAttachments, { mainRenderpass }, {});


		std::vector<VkAttachmentDescription> postAttachments(1);
		if (m_useImGui) {
			postAttachments[0] = vr::getColorAttachment(m_pOutputTexture->format, m_pOutputTexture->sampleCount, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
		else {
			postAttachments[0] = m_renderer->getSwapchainAttachment(m_pWindow->getSwapchainID());
		}

		std::vector<VkAttachmentReference> postPassReferences(1);
		postPassReferences[0].attachment = 0; // output / swapchain
		postPassReferences[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;



		std::vector<VkSubpassDescription> postRenderpasses(1);
		postRenderpasses[0].flags = 0;
		postRenderpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		postRenderpasses[0].inputAttachmentCount = 0;
		postRenderpasses[0].preserveAttachmentCount = 0;

		postRenderpasses[0].colorAttachmentCount = 1;
		postRenderpasses[0].pColorAttachments = &postPassReferences[0];
		postRenderpasses[0].pDepthStencilAttachment = nullptr;
		postRenderpasses[0].pResolveAttachments = nullptr;

		m_postRenderpass = m_renderer->createRenderPass(postAttachments, postRenderpasses, {});

		if (m_useImGui) {

			std::vector<VkAttachmentDescription> imguiAttachments(1);
			imguiAttachments[0] = m_renderer->getSwapchainAttachment(m_pWindow->getSwapchainID());


			std::vector<VkAttachmentReference> imguiPassReferences(1);
			imguiPassReferences[0].attachment = 0; // swapchain
			imguiPassReferences[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;



			std::vector<VkSubpassDescription> imguiRenderpasses(1);
			imguiRenderpasses[0].flags = 0;
			imguiRenderpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			imguiRenderpasses[0].inputAttachmentCount = 0;
			imguiRenderpasses[0].preserveAttachmentCount = 0;

			imguiRenderpasses[0].colorAttachmentCount = 1;
			imguiRenderpasses[0].pColorAttachments = &imguiPassReferences[0];
			imguiRenderpasses[0].pDepthStencilAttachment = nullptr;
			imguiRenderpasses[0].pResolveAttachments = nullptr;

			m_imguiRenderpass = m_renderer->createRenderPass(imguiAttachments, imguiRenderpasses, {});
		}
	}

	void ForwardRenderer::createFramebuffers(VkExtent2D extent)
	{
		std::vector<vr::Texture*> additionalAttachments = { m_pMainColorTexture, m_pDepthTexture };

		m_mainFramebuffer = m_renderer->createFramebuffer(m_mainRenderPass, extent, additionalAttachments);

		if (m_useImGui) {
			m_postFramebuffer = m_renderer->createFramebuffer(m_postRenderpass, extent, { m_pOutputTexture });
			
			m_imguiFramebuffer = m_renderer->createSwapchainFramebuffer(m_pWindow->getSwapchainID(), m_imguiRenderpass, {});
			m_renderer->initImGUI(m_pWindow->getWindowHandle(), m_imguiRenderpass, 0);
		}
		else {
			m_postFramebuffer = m_renderer->createSwapchainFramebuffer(m_pWindow->getSwapchainID(), m_postRenderpass, {});
		}
	}

	void ForwardRenderer::createPipelines(VkExtent2D extent) {
		vbl::PipelineConfig pipelineConfig = {};
		pipelineConfig.dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);

		m_colorPipeline = m_renderer->createPipeline(extent, m_pColorShaders, m_mainRenderPass, 0, pipelineConfig);
		m_postProcessPipline = m_renderer->createPipeline(extent, m_pPostProcessShaders, m_postRenderpass, 0, pipelineConfig);
	}

	ForwardRenderer::ForwardRenderer() {

	}

	void ForwardRenderer::swapchainResizedCallback(uint32_t width, uint32_t height) {
		std::cout << "resized: " << width << ", " << height << std::endl;
		m_renderer->waitDeviceIdle();

		//TODO: should this be done here?
		m_renderer->recreateSwapchain(m_pWindow->getSwapchainID());
		VkExtent2D extent = { width, height };
		createTextures(extent);
		createRenderPasses();
		createFramebuffers(extent);
		createPipelines(extent);

		VkImageLayout layout = m_pMainColorTexture->layout;
		m_pMainColorTexture->layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		m_renderer->updateDescriptorSet(m_pInputDescriptorSet, 0, nullptr, m_pMainColorTexture, m_sampler, true);
		m_pMainColorTexture->layout = layout;

	}

	void ForwardRenderer::init(sa::RenderWindow* pWindow, bool setupImGui) {
		m_renderer = &vr::Renderer::get();
		m_useImGui = setupImGui;

		m_pWindow = pWindow;
		
		// Setup callback
		m_renderer->setOnSwapchainResizeCallback(m_pWindow->getSwapchainID(), 
			std::bind(&ForwardRenderer::swapchainResizedCallback, this, std::placeholders::_1, std::placeholders::_2));

		// Get Extent
		sa::Vector2u windowExtent = m_pWindow->getCurrentExtent();
		VkExtent2D extent = { windowExtent.x, windowExtent.y };
		
		// Create pipeline resources
		createTextures(extent);
		createRenderPasses();
		createFramebuffers(extent);


		vr::ShaderPtr vertexShader = m_renderer->createShader("../Engine/shaders/Texture.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
		vr::ShaderPtr fragmentShader = m_renderer->createShader("../Engine/shaders/Texture.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
		m_pColorShaders = m_renderer->createShaderSet(vertexShader, fragmentShader);

		vr::ShaderPtr postProcessVertexShader = m_renderer->createShader("../Engine/shaders/PostProcess.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
		vr::ShaderPtr postProcessFragmentShader = m_renderer->createShader("../Engine/shaders/PostProcess.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
		m_pPostProcessShaders = m_renderer->createShaderSet(postProcessVertexShader, postProcessFragmentShader);

		createPipelines(extent);
		

		// Buffers DescriptorSets
		PerFrameBuffer perFrame = {};
		glm::mat4 proj = glm::perspective(glm::radians(60.f), 1000.f / 600.f, 0.001f, 100.0f);
		glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 1), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		perFrame.projViewMatrix = proj * view;
		m_pPerFrameBuffer = m_renderer->createUniformBuffer(sizeof(PerFrameBuffer), &perFrame);

		m_pPerFrameDescriptorSet = m_pColorShaders->getDescriptorSet(SET_PER_FRAME);
		m_renderer->updateDescriptorSet(m_pPerFrameDescriptorSet, 0, m_pPerFrameBuffer, nullptr, nullptr, true);
		
		//m_pLightBuffer = m_renderer->createUniformBuffer(sizeof(Light) * 64, nullptr);
		//m_renderer->updateDescriptorSet(m_pPerFrameDescriptorSet, 1, m_pLightBuffer, nullptr, nullptr, true);

		// Sampler
		m_sampler = m_renderer->createSampler(VK_FILTER_LINEAR);

		// Texture DescriptorSets
		m_pInputDescriptorSet = m_pPostProcessShaders->getDescriptorSet(0);
		VkImageLayout layout = m_pMainColorTexture->layout;
		m_pMainColorTexture->layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		m_renderer->updateDescriptorSet(m_pInputDescriptorSet, 0, nullptr, m_pMainColorTexture, m_sampler, true);
		m_pMainColorTexture->layout = layout;

	}

	void ForwardRenderer::cleanup() {

		m_pColorShaders.reset();
		m_pPostProcessShaders.reset();
	
		m_pPerFrameDescriptorSet.reset();
		m_pInputDescriptorSet.reset();

		m_sampler.reset();
		if (m_useImGui) {
			m_renderer->cleanupImGUI();
		}
	}

	void ForwardRenderer::beginFrameImGUI() {
		if (m_useImGui) {
			m_renderer->newFrameImGUI();
		}
	}

	void ForwardRenderer::draw(Scene* scene) {
		if (!m_pWindow->frame()) {
			if(m_useImGui) ImGui::EndFrame();
			return;
		}


		m_renderer->beginRenderPass(m_mainRenderPass, m_mainFramebuffer, VK_SUBPASS_CONTENTS_INLINE);
		m_renderer->bindPipeline(m_colorPipeline);

		if(scene != nullptr) {
			for (const auto& camera : scene->getActiveCameras()) {
				VkViewport viewport;
				Rect r = camera->getViewport();
				glm::vec2 pos = r.getPosition();
				glm::vec2 size = r.getSize();
				viewport.x = pos.x;
				viewport.y = pos.y;
				viewport.width = size.x;
				viewport.height = size.y;
				viewport.maxDepth = 1.0f;
				viewport.minDepth = 0.0f;

				PerFrameBuffer perFrame;
				perFrame.projViewMatrix = camera->getProjectionMatrix() * camera->getViewMatrix();
				memcpy(m_pPerFrameBuffer->mappedData, &perFrame, sizeof(perFrame));

				m_renderer->updateDescriptorSet(m_pPerFrameDescriptorSet, 0, m_pPerFrameBuffer, nullptr, nullptr, false);
				m_renderer->bindDescriptorSet(m_pPerFrameDescriptorSet, m_colorPipeline);
				m_renderer->bindViewport(viewport);
			
				scene->forEach<comp::Transform, comp::Model>([&](const comp::Transform& transform, comp::Model& modelComp) {
					
					if (modelComp.modelID == NULL_RESOURCE) {
						return; // does not have to be drawn
					}

					if (modelComp.descriptorSet == nullptr) {
						modelComp.descriptorSet = m_pColorShaders->getDescriptorSet(SET_PER_OBJECT);
					}


					sa::ModelData* model = sa::ResourceManager::get().getModel(modelComp.modelID);
					
					sa::PerObjectBuffer perObject = {};
					perObject.worldMatrix = glm::mat4(1);
					perObject.worldMatrix = glm::translate(perObject.worldMatrix, transform.position);
					perObject.worldMatrix = glm::rotate(perObject.worldMatrix, transform.rotation.x, glm::vec3(1, 0, 0));
					perObject.worldMatrix = glm::rotate(perObject.worldMatrix, transform.rotation.y, glm::vec3(0, 1, 0));
					perObject.worldMatrix = glm::rotate(perObject.worldMatrix, transform.rotation.z, glm::vec3(0, 0, 1));
					perObject.worldMatrix = glm::scale(perObject.worldMatrix, transform.scale);

					if (!modelComp.buffer.isValid()) {
						modelComp.buffer = sa::Buffer(sa::BufferType::UNIFORM, sizeof(sa::PerObjectBuffer), 1, &perObject);
					}
					else {
						modelComp.buffer.write(perObject);
					}
					m_renderer->updateDescriptorSet(modelComp.descriptorSet, 0, (const vr::Buffer*)modelComp.buffer, nullptr, nullptr, false);
					m_renderer->bindDescriptorSet(modelComp.descriptorSet, m_colorPipeline);

					for (const auto& mesh : model->meshes) {
						m_renderer->bindVertexBuffer((const vr::Buffer*)mesh.vertexBuffer);
						if (mesh.indexBuffer.isValid()) {
							m_renderer->bindIndexBuffer((const vr::Buffer*)mesh.indexBuffer);
							m_renderer->drawIndexed(mesh.indexBuffer.getElementCount(), 1);
						}
						else {
							m_renderer->draw(mesh.vertexBuffer.getElementCount(), 1);
						}
					}

				});					
			
			
			}

		}

		
		//m_renderer->nextSubpass(VK_SUBPASS_CONTENTS_INLINE);
		m_renderer->endRenderPass();
		m_renderer->beginRenderPass(m_postRenderpass, m_postFramebuffer, VK_SUBPASS_CONTENTS_INLINE);
		m_renderer->bindPipeline(m_postProcessPipline);
		m_renderer->bindDescriptorSet(m_pInputDescriptorSet, m_postProcessPipline);
		m_renderer->draw(6, 1);
		
		if (m_useImGui) {
			m_renderer->endRenderPass();
			m_renderer->beginRenderPass(m_imguiRenderpass, m_imguiFramebuffer, VK_SUBPASS_CONTENTS_INLINE);
			m_renderer->endFrameImGUI();
		}
		m_renderer->endRenderPass();

		m_pWindow->display(true);

	}

	sa::Texture ForwardRenderer::getOutputTexture() const {
		return m_pOutputTexture;
	}

	sa::Texture ForwardRenderer::createShaderTexture2D(const sa::Image& img) {
		return m_renderer->createTexture2D(m_mainFramebuffer, m_mainRenderPass, 0, { img.getExtent().x, img.getExtent().y }, img.getPixels(), img.getChannelCount());
	}
}