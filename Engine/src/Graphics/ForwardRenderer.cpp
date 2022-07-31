#include "pch.h"
#include "ForwardRenderer.h"

#include <Renderer.hpp>

namespace sa {
	
	void ForwardRenderer::createTextures(sa::Extent extent) {
		m_mainColorTexture = m_renderer.createTexture2D(sa::TextureTypeFlagBits::COLOR_ATTACHMENT | sa::TextureTypeFlagBits::SAMPLED, extent);
		m_depthTexture = m_renderer.createTexture2D(sa::TextureTypeFlagBits::DEPTH_ATTACHMENT, extent);

		m_outputTexture = m_renderer.createTexture2D(sa::TextureTypeFlagBits::COLOR_ATTACHMENT | sa::TextureTypeFlagBits::SAMPLED, extent);

		m_brightnessTexture = m_renderer.createTexture2D(sa::TextureTypeFlagBits::COLOR_ATTACHMENT | sa::TextureTypeFlagBits::STORAGE, extent);

		m_blurredBrightnessTexture = m_renderer.createTexture2D(sa::TextureTypeFlagBits::COLOR_ATTACHMENT | sa::TextureTypeFlagBits::STORAGE, extent);

	}

	void ForwardRenderer::createRenderPasses() {

		m_colorRenderProgram = m_renderer.createRenderProgram()
			.addColorAttachment(true, m_mainColorTexture)
			.addDepthAttachment(m_depthTexture)
			.addColorAttachment(true, m_brightnessTexture)
			.beginSubpass()
				.addAttachmentReference(0, sa::SubpassAttachmentUsage::ColorTarget)
				.addAttachmentReference(1, sa::SubpassAttachmentUsage::DepthTarget)
				.addAttachmentReference(2, sa::SubpassAttachmentUsage::ColorTarget)
			.endSubpass()
			.end();
			


		if (m_useImGui) {
			m_imguiRenderProgram = m_renderer.createRenderProgram()
				.addSwapchainAttachment(m_pWindow->getSwapchainID())
				.beginSubpass()
				.addAttachmentReference(0, sa::SubpassAttachmentUsage::ColorTarget)
				.endSubpass()
				.end();
		}

	}

	void ForwardRenderer::createFramebuffers(sa::Extent extent)
	{
		
		m_colorFramebuffer = m_renderer.createFramebuffer(m_colorRenderProgram, { m_mainColorTexture, m_depthTexture, m_brightnessTexture });

		if (m_useImGui) {
			m_postFramebuffer = m_renderer.createFramebuffer(m_postRenderProgram, { m_outputTexture });

			m_imguiFramebuffer = m_renderer.createSwapchainFramebuffer(m_imguiRenderProgram, m_pWindow->getSwapchainID(), {});
			m_renderer.initImGui(*m_pWindow, m_imguiRenderProgram, 0);

		}
		else {
			m_postFramebuffer = m_renderer.createSwapchainFramebuffer(m_postRenderProgram, m_pWindow->getSwapchainID(), {});
		}
	}

	void ForwardRenderer::updateMaterial(Material* pMaterial) {
		if (pMaterial->valueBuffer.isValid()) {
			pMaterial->valueBuffer.write(pMaterial->values);
			m_renderer.updateDescriptorSet(pMaterial->descriptorSet, 0, pMaterial->valueBuffer);
		}

		if(pMaterial->sampler != NULL_RESOURCE)
			m_renderer.updateDescriptorSet(pMaterial->descriptorSet, 1, pMaterial->sampler);
		
		if(pMaterial->diffuseMap.isValid())
			m_renderer.updateDescriptorSet(pMaterial->descriptorSet, 2, pMaterial->diffuseMap);

		if (pMaterial->normalMap.isValid())
			m_renderer.updateDescriptorSet(pMaterial->descriptorSet, 3, pMaterial->normalMap);

		if (pMaterial->specularMap.isValid())
			m_renderer.updateDescriptorSet(pMaterial->descriptorSet, 4, pMaterial->specularMap);


	}

	void ForwardRenderer::bindMaterial(RenderContext& context, Material* pMaterial) {
		context.bindDescriptorSet(pMaterial->descriptorSet, pMaterial->pipeline);
	}

	ForwardRenderer::ForwardRenderer() 
		: m_renderer(sa::Renderer::get()) 
	{

	}

	void ForwardRenderer::swapchainResizedCallback(Extent extent) {
		std::cout << "resized: " << extent.width << ", " << extent.height << std::endl;
		

		createTextures(extent);
		createRenderPasses();
		createFramebuffers(extent);
		


		m_colorPipeline = m_renderer.createGraphicsPipeline(m_colorRenderProgram, 0, extent,
			"../Engine/shaders/Texture.vert.spv", "../Engine/shaders/Texture.frag.spv");

		m_postProcessPipeline = m_renderer.createGraphicsPipeline(m_postRenderProgram, 0, extent,
			"../Engine/shaders/PostProcess.vert.spv", "../Engine/shaders/PostProcess.frag.spv");

		m_renderer.updateDescriptorSet(m_blurDescriptorSet, 0, m_brightnessTexture);
		m_renderer.updateDescriptorSet(m_blurDescriptorSet, 1, m_blurredBrightnessTexture);

		m_renderer.updateDescriptorSet(m_postInputDescriptorSet, 0, m_mainColorTexture, m_sampler);

	}

	void ForwardRenderer::init(sa::RenderWindow* pWindow, bool setupImGui) {
		m_useImGui = setupImGui;

		m_pWindow = pWindow;
		
		// Setup callback
		pWindow->setResizeCallback(std::bind(&ForwardRenderer::swapchainResizedCallback, this, std::placeholders::_1));

		// Get Extent
		sa::Extent windowExtent = m_pWindow->getCurrentExtent();
		
		// Create pipeline resources
		createTextures(windowExtent);
		createRenderPasses();

		auto factory = m_renderer.createRenderProgram();
		if (m_useImGui) {
			factory.addColorAttachment(true, m_outputTexture);
		}
		else {
			factory.addSwapchainAttachment(m_pWindow->getSwapchainID());
		}

		m_postRenderProgram = factory.beginSubpass()
			.addAttachmentReference(0, sa::SubpassAttachmentUsage::ColorTarget)
			.endSubpass()
			.end();

		createFramebuffers(windowExtent);


		m_colorPipeline = m_renderer.createGraphicsPipeline(m_colorRenderProgram, 0, windowExtent,
			"../Engine/shaders/ForwardColorPass.vert.spv", "../Engine/shaders/ForwardColorPass.frag.spv");

		m_postProcessPipeline = m_renderer.createGraphicsPipeline(m_postRenderProgram, 0, windowExtent,
			"../Engine/shaders/PostProcess.vert.spv", "../Engine/shaders/PostProcess.frag.spv");

		m_blurPipeline = m_renderer.createComputePipeline("../Engine/shaders/GaussianBlur.comp.spv");


		// BLUR PASS
		m_blurDescriptorSet =  m_renderer.allocateDescriptorSet(m_blurPipeline, 0);
		m_renderer.updateDescriptorSet(m_blurDescriptorSet, 0, m_brightnessTexture);
		m_renderer.updateDescriptorSet(m_blurDescriptorSet, 1, m_blurredBrightnessTexture);



		m_blurContext = m_renderer.createSubContext();
		m_blurContext.preRecord([=](sa::RenderContext& context) {
			context.bindPipeline(m_blurPipeline);
			context.bindDescriptorSet(m_blurDescriptorSet, m_blurPipeline);
			context.dispatch(32, 32, 1);
		}, (sa::ContextUsageFlags)0);
		


		// Buffers DescriptorSets
		PerFrameBuffer perFrame = {};
		glm::mat4 proj = glm::perspective(glm::radians(60.f), 1000.f / 600.f, 0.001f, 100.0f);
		glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 1), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		perFrame.projViewMatrix = proj * view;
		m_perFrameBuffer = m_renderer.createBuffer(sa::BufferType::UNIFORM, sizeof(PerFrameBuffer), &perFrame);

		m_perFrameDescriptorSet = m_renderer.allocateDescriptorSet(m_colorPipeline, SET_PER_FRAME);
		m_renderer.updateDescriptorSet(m_perFrameDescriptorSet, 0, m_perFrameBuffer);


		//m_pLightBuffer = m_renderer->createUniformBuffer(sizeof(Light) * 64, nullptr);
		//m_renderer->updateDescriptorSet(m_pPerFrameDescriptorSet, 1, m_pLightBuffer, nullptr, nullptr, true);

		// Sampler
		m_sampler = m_renderer.createSampler(sa::FilterMode::LINEAR);

		// Texture DescriptorSets
		m_postInputDescriptorSet = m_renderer.allocateDescriptorSet(m_postRenderProgram, 0);
		m_renderer.updateDescriptorSet(m_postInputDescriptorSet, 0, m_mainColorTexture, m_sampler);
		
		// DEBUG
		m_matID = AssetManager::get().newMaterial(m_colorPipeline);
		Material* mat = AssetManager::get().getMaterial(m_matID);

		sa::Image img("../Box.png");
		mat->diffuseMap = m_renderer.createTexture2D(img, true);
		mat->sampler = m_sampler;
		mat->values.diffuseColor = { 1.0f, 0.3f, 0.3f, 1.0f };
		updateMaterial(mat);

	}

	void ForwardRenderer::cleanup() {

	}

	void ForwardRenderer::beginFrameImGUI() {
		if (m_useImGui) {
			m_renderer.newImGuiFrame();
		}
	}

	void ForwardRenderer::draw(Scene* scene) {
		timer += 0.01f;

		/*
		unsigned char pixels[] = {
			0, (std::sin(timer) + 1) * 0.5f * 255, 255, 255
		};
		m_renderer->updateTexture(m_defaultTexture, m_mainFramebuffer, m_mainRenderPass, 0, pixels, 4);
		*/		
		



		sa::RenderContext context = m_pWindow->beginFrame();
		if (!context) {
			if(m_useImGui) ImGui::EndFrame();
			return;
		}

		
		//context.executeSubContext(m_blurContext);

		context.beginRenderProgram(m_colorRenderProgram, m_colorFramebuffer, sa::SubpassContents::DIRECT);
		context.bindPipeline(m_colorPipeline);


		if(scene != nullptr) {
			for (const auto& camera : scene->getActiveCameras()) {
				
				PerFrameBuffer perFrame;
				perFrame.projViewMatrix = camera->getProjectionMatrix() * camera->getViewMatrix();
				m_perFrameBuffer.write(perFrame);
				context.updateDescriptorSet(m_perFrameDescriptorSet, 0, m_perFrameBuffer);

				context.bindDescriptorSet(m_perFrameDescriptorSet, m_colorPipeline);
				
				Material* mat = AssetManager::get().getMaterial(m_matID);
				bindMaterial(context, mat);

				scene->forEach<comp::Transform, comp::Model>([&](const comp::Transform& transform, comp::Model& modelComp) {
					
					if (modelComp.modelID == NULL_RESOURCE) {
						return; // does not have to be drawn
					}

					if (modelComp.descriptorSet == NULL_RESOURCE) {
						modelComp.descriptorSet = m_renderer.allocateDescriptorSet(m_colorPipeline, SET_PER_OBJECT);
					}


					sa::ModelData* model = sa::AssetManager::get().getModel(modelComp.modelID);
					
					sa::PerObjectBuffer perObject = {};
					perObject.worldMatrix = glm::mat4(1);
					
					perObject.worldMatrix = glm::translate(perObject.worldMatrix, transform.position);
					perObject.worldMatrix = glm::rotate(perObject.worldMatrix, transform.rotation.x, glm::vec3(1, 0, 0));
					perObject.worldMatrix = glm::rotate(perObject.worldMatrix, transform.rotation.y, glm::vec3(0, 1, 0));
					perObject.worldMatrix = glm::rotate(perObject.worldMatrix, transform.rotation.z, glm::vec3(0, 0, 1));
					perObject.worldMatrix = glm::scale(perObject.worldMatrix, transform.scale);
				

					context.pushConstant(m_colorPipeline, sa::ShaderStageFlagBits::VERTEX, perObject.worldMatrix);

					for (const auto& mesh : model->meshes) {
						context.bindVertexBuffers(0, { mesh.vertexBuffer });
						if (mesh.indexBuffer.isValid()) {
							context.bindIndexBuffer(mesh.indexBuffer);
							context.drawIndexed(mesh.indexBuffer.getElementCount<uint32_t>(), 1);
						}
						else {
							context.draw(mesh.vertexBuffer.getElementCount<VertexUV>(), 1);
						}
					}

				});					
				
				/*
				scene->forEach<comp::Transform, comp::Model>([&](const comp::Transform& transform, const comp::Model& model) {
					sa::ModelData* modelData = sa::AssetManager::get().getModel(model.modelID);
					Matrix4x4 worldMatrix = Matrix4x4(1);
					worldMatrix = glm::translate(worldMatrix, transform.position);
					worldMatrix = glm::rotate(worldMatrix, transform.rotation.x, Vector3(1, 0, 0));
					worldMatrix = glm::rotate(worldMatrix, transform.rotation.y, Vector3(0, 1, 0));
					worldMatrix = glm::rotate(worldMatrix, transform.rotation.z, Vector3(0, 0, 1));
					worldMatrix = glm::scale(worldMatrix, transform.scale);
					for (auto &mesh : modelData->meshes) {
						auto [element, success] = m_meshPositions.insert({ &mesh, worldMatrix });

						if (success) {
							m_materialMeshes[m_matID].push_back(&mesh);
						}

					}

				});

				for (auto [materialID, meshes] : m_materialMeshes) {
					Material* mat = AssetManager::get().getMaterial(materialID);
					bindMaterial(context, mat);

					for (auto mesh : meshes) {
					context.pushConstant(m_colorPipeline, sa::ShaderStageFlagBits::VERTEX, m_meshPositions[mesh]);

						context.bindVertexBuffers(0, { mesh->vertexBuffer });
						if (mesh->indexBuffer.isValid()) {
							context.bindIndexBuffer(mesh->indexBuffer);
							context.drawIndexed(mesh->indexBuffer.getElementCount<uint32_t>(), 1);
						}
						else {
							context.draw(mesh->vertexBuffer.getElementCount<VertexUV>(), 1);
						}
					}
				}
			
				*/
			}

		}

		
		context.endRenderProgram(m_colorRenderProgram);

		context.beginRenderProgram(m_postRenderProgram, m_postFramebuffer, sa::SubpassContents::DIRECT);
		context.bindPipeline(m_postProcessPipeline);
		context.bindDescriptorSet(m_postInputDescriptorSet, m_postProcessPipeline);
		context.draw(6, 1);
		
		ResourceID renderProgram = m_postRenderProgram;

		if (m_useImGui) {
			context.endRenderProgram(m_postRenderProgram);
			context.beginRenderProgram(m_imguiRenderProgram, m_imguiFramebuffer, sa::SubpassContents::DIRECT);
			context.renderImGuiFrame();
			renderProgram = m_imguiRenderProgram;
		}
		context.endRenderProgram(renderProgram);

		m_pWindow->display();

	}

	sa::Texture ForwardRenderer::getOutputTexture() const {
		return m_outputTexture;
	}

}