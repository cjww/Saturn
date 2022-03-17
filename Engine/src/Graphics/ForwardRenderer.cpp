#include "pch.h"
#include "ForwardRenderer.h"

#include "Graphics\Vulkan\Renderer.hpp"

namespace sa {

	ForwardRenderer::ForwardRenderer() {

	}

	void ForwardRenderer::init(sa::RenderWindow* pWindow, bool setupImGui) {
		m_renderer = &vr::Renderer::get();
		m_useImGui = setupImGui;

		m_pWindow = pWindow;

		sa::Vector2u windowExtent = pWindow->getCurrentExtent();

		VkExtent2D extent = { windowExtent.x, windowExtent.y };
		

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

		
		std::vector<VkAttachmentDescription> attachments(3);

		attachments[0] = m_renderer->getSwapchainAttachment(m_pWindow->getSwapchainID());
		attachments[1] = vr::getColorAttachment(m_pMainColorTexture->format, m_pMainColorTexture->sampleCount);
		attachments[2] = vr::getDepthAttachment(m_pDepthTexture->format, m_pDepthTexture->sampleCount);
		
		if (m_useImGui) {
			attachments.push_back(vr::getColorAttachment(m_pOutputTexture->format, m_pOutputTexture->sampleCount));
		}

		//attachments[3] = vr::getResolveAttachment(m_pMainColorTexture->format);
		


		std::vector<VkAttachmentReference> firstPassReferences(2);
		firstPassReferences[0].attachment = 1; // m_pMainColorTexture / output
		firstPassReferences[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		firstPassReferences[1].attachment = 2; // m_pDepthTexture
		firstPassReferences[1].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		std::vector<VkAttachmentReference> secondPassReferences(2);
		secondPassReferences[0].attachment = (m_useImGui)? 3 : 0; // output or swapchain
		secondPassReferences[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		secondPassReferences[1].attachment = 1; // m_pMainColorTexture
		secondPassReferences[1].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		
		std::vector<VkAttachmentReference> thirdPassReference(2);
		thirdPassReference[0].attachment = 0; // swapchain
		thirdPassReference[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		thirdPassReference[1].attachment = 3;
		thirdPassReference[1].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		
		/*
		VkSubpassDescription mainSubpass;
		mainSubpass.flags = 0;
		mainSubpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		mainSubpass.inputAttachmentCount = 0;
		mainSubpass.preserveAttachmentCount = 0;

		mainSubpass.colorAttachmentCount = 1;
		mainSubpass.pColorAttachments = &firstPassReferences[0];
		mainSubpass.pDepthStencilAttachment = &firstPassReferences[1];
		*/


		std::vector<VkSubpassDescription> subpasses(2);
		subpasses[0].flags = 0;
		subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpasses[0].inputAttachmentCount = 0;
		subpasses[0].preserveAttachmentCount = 0;

		subpasses[0].colorAttachmentCount = 1;
		subpasses[0].pColorAttachments = &firstPassReferences[0];
		subpasses[0].pDepthStencilAttachment = &firstPassReferences[1];
		
		subpasses[1].flags = 0;
		subpasses[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpasses[1].inputAttachmentCount = 1;
		subpasses[1].pInputAttachments = &secondPassReferences[1];
		subpasses[1].preserveAttachmentCount = 0;

		subpasses[1].colorAttachmentCount = 1;
		subpasses[1].pColorAttachments = &secondPassReferences[0];
		subpasses[1].pDepthStencilAttachment = nullptr;
		if (m_useImGui) {
			
			subpasses.resize(3);
			subpasses[2].flags = 0;
			subpasses[2].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpasses[2].inputAttachmentCount = 1;
			subpasses[2].pInputAttachments = &thirdPassReference[1];
			subpasses[2].preserveAttachmentCount = 0;
		
			subpasses[2].colorAttachmentCount = 1;
			subpasses[2].pColorAttachments = &thirdPassReference[0];
			subpasses[2].pDepthStencilAttachment = nullptr;
			
		}

		std::vector<VkSubpassDependency> subpassDependencies(1);
		subpassDependencies[0].dependencyFlags = 0;
		subpassDependencies[0].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
		subpassDependencies[0].srcSubpass = 0;
		subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
		subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
		subpassDependencies[0].dstSubpass = 1;

		if (m_useImGui) {
			subpassDependencies.resize(2);
			subpassDependencies[1].dependencyFlags = 0;
			subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
			subpassDependencies[1].srcSubpass = 1;
			subpassDependencies[1].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
			subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
			subpassDependencies[1].dstSubpass = 2;
		}
		

		m_mainRenderPass = m_renderer->createRenderPass(attachments, subpasses, subpassDependencies);
		
		std::vector<vr::Texture*> additionalAttachments = { m_pMainColorTexture, m_pDepthTexture };
		
		if (m_useImGui) {
			additionalAttachments.push_back(m_pOutputTexture);
			m_renderer->initImGUI(pWindow->getWindowHandle(), m_mainRenderPass, 2);
		}
		

		m_mainFramebuffer = m_renderer->createSwapchainFramebuffer(m_pWindow->getSwapchainID(), m_mainRenderPass, additionalAttachments);
		//m_mainFramebuffer = m_renderer->createFramebuffer(m_renderPass, extent, additionalAttachments);


		vr::ShaderPtr vertexShader = m_renderer->createShader("../Engine/shaders/Texture.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
		vr::ShaderPtr fragmentShader = m_renderer->createShader("../Engine/shaders/Texture.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
		m_pColorShaders = m_renderer->createShaderSet(vertexShader, fragmentShader);

		vr::ShaderPtr postProcessVertexShader = m_renderer->createShader("../Engine/shaders/PostProcess.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
		vr::ShaderPtr postProcessFragmentShader = m_renderer->createShader("../Engine/shaders/PostProcess.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
		m_pPostProcessShaders = m_renderer->createShaderSet(postProcessVertexShader, postProcessFragmentShader);

		vbl::PipelineConfig pipelineConfig = {};
		pipelineConfig.dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);

		m_colorPipeline = m_renderer->createPipeline(extent, m_pColorShaders, m_mainRenderPass, 0, pipelineConfig);
		m_postProcessPipline = m_renderer->createPipeline(extent, m_pPostProcessShaders, m_mainRenderPass, 1, pipelineConfig);
		

		PerFrameBuffer perFrame = {};
		glm::mat4 proj = glm::perspective(glm::radians(60.f), 1000.f / 600.f, 0.001f, 100.0f);
		glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 1), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		perFrame.projViewMatrix = proj * view;
		m_pPerFrameBuffer = m_renderer->createUniformBuffer(sizeof(PerFrameBuffer), &perFrame);

		//m_pLightBuffer = m_renderer->createUniformBuffer(sizeof(Light) * 64, nullptr);

		m_pPerFrameDescriptorSet = m_pColorShaders->getDescriptorSet(SET_PER_FRAME);
		m_renderer->updateDescriptorSet(m_pPerFrameDescriptorSet, 0, m_pPerFrameBuffer, nullptr, nullptr, true);
		
		//m_renderer->updateDescriptorSet(m_pPerFrameDescriptorSet, 1, m_pLightBuffer, nullptr, nullptr, true);
		m_pInputDescriptorSet = m_pPostProcessShaders->getDescriptorSet(0);
		VkImageLayout layout = m_pMainColorTexture->layout;
		m_pMainColorTexture->layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		m_renderer->updateDescriptorSet(m_pInputDescriptorSet, 0, nullptr, m_pMainColorTexture, nullptr, true);
		m_pMainColorTexture->layout = layout;

	}

	void ForwardRenderer::cleanup() {

		m_pColorShaders.reset();
		m_pPostProcessShaders.reset();
	
		m_pPerFrameDescriptorSet.reset();
		m_pInputDescriptorSet.reset();

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
		m_pWindow->frame();


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

		
		m_renderer->nextSubpass(VK_SUBPASS_CONTENTS_INLINE);
		
		m_renderer->bindPipeline(m_postProcessPipline);
		m_renderer->bindDescriptorSet(m_pInputDescriptorSet, m_postProcessPipline);

		m_renderer->draw(6, 1);
		

		if (m_useImGui) {
			m_renderer->nextSubpass(VK_SUBPASS_CONTENTS_INLINE);
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