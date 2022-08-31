#include "pch.h"
#include "ForwardRenderer.h"

#include <Renderer.hpp>

namespace sa {
	
	void ForwardRenderer::setupColorPass(Extent extent) {
		SA_PROFILE_FUNCTION();

		m_mainColorTexture = m_renderer.createTexture2D(sa::TextureTypeFlagBits::COLOR_ATTACHMENT | sa::TextureTypeFlagBits::SAMPLED, extent);
		m_depthTexture = m_renderer.createTexture2D(sa::TextureTypeFlagBits::DEPTH_ATTACHMENT, extent);
		m_brightnessTexture = m_renderer.createTexture2D(sa::TextureTypeFlagBits::COLOR_ATTACHMENT | sa::TextureTypeFlagBits::STORAGE | sa::TextureTypeFlagBits::SAMPLED, extent);


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


		m_colorFramebuffer = m_renderer.createFramebuffer(m_colorRenderProgram, { m_mainColorTexture, m_depthTexture, m_brightnessTexture });


		m_colorPipeline = m_renderer.createGraphicsPipeline(m_colorRenderProgram, 0, extent,
			"../Engine/shaders/ForwardColorPass.vert.spv", "../Engine/shaders/ForwardColorPass.frag.spv");

		m_perFrameDescriptorSet = m_renderer.allocateDescriptorSet(m_colorPipeline, SET_PER_FRAME);


		PerFrameBuffer perFrame = {};
		perFrame.projViewMatrix = sa::Matrix4x4(1);
		perFrame.viewPos = sa::Vector3(0);
		m_perFrameBuffer = m_renderer.createBuffer(sa::BufferType::UNIFORM, sizeof(PerFrameBuffer), &perFrame);

		m_renderer.updateDescriptorSet(m_perFrameDescriptorSet, 0, m_perFrameBuffer);

	}

	void ForwardRenderer::setupPostPass(Extent extent) {
		SA_PROFILE_FUNCTION();

		m_outputTexture = m_renderer.createTexture2D(sa::TextureTypeFlagBits::COLOR_ATTACHMENT | sa::TextureTypeFlagBits::SAMPLED, extent);

		auto factory = m_renderer.createRenderProgram();
		if (m_useImGui) {
			factory.addColorAttachment(true, m_outputTexture);
		}
		else {
			factory.addSwapchainAttachment(m_pWindow->getSwapchainID());
		}

		m_postRenderProgram = factory
			.beginSubpass()
			.addAttachmentReference(0, sa::SubpassAttachmentUsage::ColorTarget)
			.endSubpass()
			.addSubpassDependency()
			.end();

		if (m_useImGui) {
			m_imguiRenderProgram = m_renderer.createRenderProgram()
				.addSwapchainAttachment(m_pWindow->getSwapchainID())
				.beginSubpass()
				.addAttachmentReference(0, sa::SubpassAttachmentUsage::ColorTarget)
				.endSubpass()
				.end();
		}
	

		if (m_useImGui) {
			m_postFramebuffer = m_renderer.createFramebuffer(m_postRenderProgram, { m_outputTexture });

			m_imguiFramebuffer = m_renderer.createSwapchainFramebuffer(m_imguiRenderProgram, m_pWindow->getSwapchainID(), {});
			m_renderer.initImGui(*m_pWindow, m_imguiRenderProgram, 0);

		}
		else {
			m_postFramebuffer = m_renderer.createSwapchainFramebuffer(m_postRenderProgram, m_pWindow->getSwapchainID(), {});
		}

		m_postProcessPipeline = m_renderer.createGraphicsPipeline(m_postRenderProgram, 0, extent,
			"../Engine/shaders/PostProcess.vert.spv", "../Engine/shaders/PostProcess.frag.spv");
		
		m_postInputDescriptorSet = m_renderer.allocateDescriptorSet(m_postRenderProgram, 0);

		m_blurredBrightnessTexture = m_renderer.createTexture2D(sa::TextureTypeFlagBits::SAMPLED | sa::TextureTypeFlagBits::STORAGE, extent);
		
		m_renderer.updateDescriptorSet(m_postInputDescriptorSet, 0, m_mainColorTexture, m_sampler);
		//m_renderer.updateDescriptorSet(m_postInputDescriptorSet, 1, m_blurredBrightnessTexture, m_sampler);

	}

	void ForwardRenderer::setupBlurPass() {
		SA_PROFILE_FUNCTION();

		m_blurPipeline = m_renderer.createComputePipeline("../Engine/shaders/GaussianBlur.comp.spv");

		m_blurDescriptorSet = m_renderer.allocateDescriptorSet(m_blurPipeline, 0);

		m_renderer.updateDescriptorSet(m_blurDescriptorSet, 0, m_brightnessTexture);
		m_renderer.updateDescriptorSet(m_blurDescriptorSet, 1, m_blurredBrightnessTexture);
		
		m_blurContext = m_renderer.createSubContext();
		m_blurContext.preRecord([=](sa::RenderContext& context) {
			context.bindPipeline(m_blurPipeline);
			context.bindDescriptorSet(m_blurDescriptorSet, m_blurPipeline);
			context.dispatch(32, 32, 1);
			}, (sa::ContextUsageFlags)0);

	}

	void ForwardRenderer::swapchainResizedCallback(Extent extent) {
		std::cout << "resized: " << extent.width << ", " << extent.height << std::endl;
		
		setupColorPass(extent);
		setupPostPass(extent);

	}

	void ForwardRenderer::init(sa::RenderWindow* pWindow, bool setupImGui) {
		SA_PROFILE_FUNCTION();

		m_useImGui = setupImGui;

		m_pWindow = pWindow;
		
		// Setup callback
		pWindow->setResizeCallback(std::bind(&ForwardRenderer::swapchainResizedCallback, this, std::placeholders::_1));

		// Get Extent
		sa::Extent windowExtent = m_pWindow->getCurrentExtent();
		
		
		// Sampler
		m_sampler = m_renderer.createSampler(sa::FilterMode::LINEAR);
		
		setupColorPass(windowExtent);
		setupPostPass(windowExtent);
		setupBlurPass();

		
		m_lightBuffer = m_renderer.createBuffer(sa::BufferType::STORAGE);
		
	}

	void ForwardRenderer::cleanup() {

	}

	void ForwardRenderer::beginFrameImGUI() {
		if (m_useImGui) {
			m_renderer.newImGuiFrame();
		}
	}

	void ForwardRenderer::draw(Scene* scene) {
		SA_PROFILE_FUNCTION();

		updateLights(scene);
		
		sa::RenderContext context = m_pWindow->beginFrame();
		if (!context) {
			if(m_useImGui) ImGui::EndFrame();
			return;
		}

		
		context.beginRenderProgram(m_colorRenderProgram, m_colorFramebuffer, sa::SubpassContents::DIRECT);
		context.bindPipeline(m_colorPipeline);


		if(scene != nullptr) {

			std::unordered_map<ResourceID, std::vector<std::tuple<Mesh*, Matrix4x4>>> meshes;
			{
				SA_PROFILE_SCOPE("Collect meshes");
				scene->forEach<comp::Transform, comp::Model>([&](const comp::Transform& transform, comp::Model& modelComp) {

					if (modelComp.modelID == NULL_RESOURCE) {
						return; // does not have to be drawn
					}

					sa::ModelData* model = sa::AssetManager::get().getModel(modelComp.modelID);
				 
					for (auto& mesh : model->meshes) {
						meshes[mesh.materialID].push_back({ &mesh, transform.getMatrix()});
					}
				});
			}

			for (const auto& camera : scene->getActiveCameras()) {
				SA_PROFILE_SCOPE("Draw camera");

				PerFrameBuffer perFrame;
				perFrame.projViewMatrix = camera->getProjectionMatrix() * camera->getViewMatrix();
				perFrame.viewPos = camera->getPosition();
				m_perFrameBuffer.write(perFrame);
				context.updateDescriptorSet(m_perFrameDescriptorSet, 0, m_perFrameBuffer);

				context.bindDescriptorSet(m_perFrameDescriptorSet, m_colorPipeline);
				
				// Opaque pass
				std::unordered_map<ResourceID, std::vector<std::tuple<Mesh*, Matrix4x4>>> transparentMaterials;
				for (const auto& [materialID, pMeshes] : meshes) {
					Material* mat = AssetManager::get().getMaterial(materialID);
					if (mat->values.opacity < 1.0f) {
						transparentMaterials[materialID] = pMeshes;
						continue;
					}
					SA_PROFILE_SCOPE("Draw material: " + std::to_string(materialID));
					mat->bind(context, m_colorPipeline, m_sampler);
					for (const auto& [pMesh, matrix] : pMeshes) {
						context.pushConstant(m_colorPipeline, sa::ShaderStageFlagBits::VERTEX, matrix);

						/*
						context.bindVertexBuffers(0, { pMesh->vertexBuffer });
						if (pMesh->indexBuffer.isValid()) {
							context.bindIndexBuffer(pMesh->indexBuffer);
							context.drawIndexed(pMesh->indexBuffer.getElementCount<uint32_t>(), 1);
						}
						else {
							context.draw(pMesh->vertexBuffer.getElementCount<VertexUV>(), 1);
						}
						*/
					}
				}

				// Transparent pass
				for (const auto& [materialID, pMeshes] : transparentMaterials) {
					SA_PROFILE_SCOPE("Draw Transparent material: " + std::to_string(materialID));

					Material* mat = AssetManager::get().getMaterial(materialID);
					mat->bind(context, m_colorPipeline, m_sampler);
					for (const auto& [pMesh, matrix] : pMeshes) {
						context.pushConstant(m_colorPipeline, sa::ShaderStageFlagBits::VERTEX, matrix);
						/*
						context.bindVertexBuffers(0, { pMesh->vertexBuffer });
						if (pMesh->indexBuffer.isValid()) {
							context.bindIndexBuffer(pMesh->indexBuffer);
							context.drawIndexed(pMesh->indexBuffer.getElementCount<uint32_t>(), 1);
						}
						else {
							context.draw(pMesh->vertexBuffer.getElementCount<VertexUV>(), 1);
						}
						*/
					}
				}

			}
			

		}

		{
			SA_PROFILE_SCOPE("End renderprogram");
			context.endRenderProgram(m_colorRenderProgram);
		}

		{
		/*
			SA_PROFILE_SCOPE("Blur");
			context.transitionTexture(m_brightnessTexture, sa::Transition::RENDER_PROGRAM_OUTPUT, sa::Transition::COMPUTE_SHADER_READ);
			context.transitionTexture(m_blurredBrightnessTexture, sa::Transition::NONE, sa::Transition::COMPUTE_SHADER_WRITE);
			context.executeSubContext(m_blurContext);
			context.transitionTexture(m_blurredBrightnessTexture, sa::Transition::COMPUTE_SHADER_WRITE, sa::Transition::FRAGMENT_SHADER_READ);
			context.transitionTexture(m_brightnessTexture, sa::Transition::COMPUTE_SHADER_READ, sa::Transition::RENDER_PROGRAM_OUTPUT);
		*/
		}
		{
			SA_PROFILE_SCOPE("Post");
			context.beginRenderProgram(m_postRenderProgram, m_postFramebuffer, sa::SubpassContents::DIRECT);
			context.bindPipeline(m_postProcessPipeline);
			context.bindDescriptorSet(m_postInputDescriptorSet, m_postProcessPipeline);
			context.draw(6, 1);
		}
		
		{
			SA_PROFILE_SCOPE("End or Imgui");
			ResourceID renderProgram = m_postRenderProgram;
			if (m_useImGui) {
				context.endRenderProgram(m_postRenderProgram);
				context.beginRenderProgram(m_imguiRenderProgram, m_imguiFramebuffer, sa::SubpassContents::DIRECT);
				context.renderImGuiFrame();
				renderProgram = m_imguiRenderProgram;
			}
			context.endRenderProgram(renderProgram);
		
		}
		
		{
			SA_PROFILE_SCOPE("Display");
			m_pWindow->display();
		}

	}

	sa::Texture ForwardRenderer::getOutputTexture() const {
		return m_outputTexture;
	}

	void ForwardRenderer::updateLights(Scene* pScene) {
		SA_PROFILE_FUNCTION();

		std::vector<LightData> lights;
		pScene->forEach<comp::Light>([&](const comp::Light& light) {
			lights.push_back(light.values);
		});
		m_lightBuffer.clear();

		struct L {
			uint32_t count;
			alignas(16) LightData lights[4096];
		};
		L lightsStruct;
		lightsStruct.count = lights.size();
		memcpy(lightsStruct.lights, lights.data(), lights.size() * sizeof(LightData));

		m_lightBuffer.write(lightsStruct);
		m_renderer.updateDescriptorSet(m_perFrameDescriptorSet, 1, m_lightBuffer);
	}

}