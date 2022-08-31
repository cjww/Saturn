#include "pch.h"
#include "ForwardPlus.h"

namespace sa {


	void ForwardPlus::createTextures(sa::Extent extent) {
		if (m_colorTexture.isValid()) m_colorTexture.destroy();
		if (m_depthTexture.isValid()) m_depthTexture.destroy();
		if (m_outputTexture.isValid()) m_outputTexture.destroy();


		m_colorTexture = m_renderer.createTexture2D(sa::TextureTypeFlagBits::COLOR_ATTACHMENT | sa::TextureTypeFlagBits::SAMPLED, extent);
		m_depthTexture = m_renderer.createTexture2D(sa::TextureTypeFlagBits::DEPTH_ATTACHMENT, extent);
		
		m_outputTexture = m_renderer.createTexture2D(sa::TextureTypeFlagBits::COLOR_ATTACHMENT | sa::TextureTypeFlagBits::SAMPLED, extent);
	}

	void ForwardPlus::createRenderPasses() {
		m_colorRenderProgram = m_renderer.createRenderProgram()
			.addColorAttachment(true, m_colorTexture)
			.addDepthAttachment(m_depthTexture)
			.beginSubpass()
			.addAttachmentReference(0, sa::SubpassAttachmentUsage::ColorTarget)
			.addAttachmentReference(1, sa::SubpassAttachmentUsage::DepthTarget)
			.endSubpass()
			.end();


		auto composeProgramFactory = m_renderer.createRenderProgram();
		if (m_useImGui) {
			composeProgramFactory.addColorAttachment(true, m_outputTexture);
		}
		else {
			composeProgramFactory.addSwapchainAttachment(m_pWindow->getSwapchainID());
		}

		m_composeRenderProgram = composeProgramFactory.beginSubpass()
			.addAttachmentReference(0, SubpassAttachmentUsage::ColorTarget)
			.endSubpass()
			.addSubpassDependency()
			.end();


	}

	void ForwardPlus::createFramebuffers(sa::Extent extent) {
		if (m_colorFramebuffer != NULL_RESOURCE)
			m_renderer.destroyFramebuffer(m_colorFramebuffer);
	
		m_colorFramebuffer = m_renderer.createFramebuffer(m_colorRenderProgram, { m_colorTexture, m_depthTexture });
	
		if (m_composeFramebuffer != NULL_RESOURCE)
			m_renderer.destroyFramebuffer(m_composeFramebuffer);

		if (m_useImGui) {
			m_composeFramebuffer = m_renderer.createFramebuffer(m_composeRenderProgram, { m_outputTexture });
		}
		else {
			m_composeFramebuffer = m_renderer.createSwapchainFramebuffer(m_composeRenderProgram, m_pWindow->getSwapchainID(), {});
		}

	}

	void ForwardPlus::createPipelines(Extent extent) {
		if (m_colorPipeline != NULL_RESOURCE)
			m_renderer.destroyPipeline(m_colorPipeline);

		m_colorPipeline = m_renderer.createGraphicsPipeline(m_colorRenderProgram, 0, extent,
			"../Engine/shaders/ForwardPlusColorPass.vert.spv", "../Engine/shaders/ForwardPlusColorPass.frag.spv");
	
		if (m_composePipeline != NULL_RESOURCE)
			m_renderer.destroyPipeline(m_composePipeline);

		m_composePipeline = m_renderer.createGraphicsPipeline(m_composeRenderProgram, 0, extent,
				"../Engine/shaders/Compose.vert.spv", "../Engine/shaders/Compose.frag.spv");

	}

	std::vector<IndirectDrawData> ForwardPlus::collectDrawData(Scene* pScene) const {
		SA_PROFILE_FUNCTION();

		std::vector<IndirectDrawData> draws;
		unsigned int lastOpaqueIndex = 0;

		pScene->forEach<comp::Transform, comp::Model>([&](const comp::Transform& transform, comp::Model& modelComp) {
			if (modelComp.modelID == NULL_RESOURCE)
				return; // does not have to be drawn
			
			// Get Model
			sa::ModelData* pModel = sa::AssetManager::get().getModel(modelComp.modelID);

			for (auto& mesh : pModel->meshes) {
				Material* pMaterial = AssetManager::get().getMaterial(mesh.materialID);
				bool found = false;
				for (auto& draw : draws) {
					if (draw.pMesh == &mesh && draw.pMaterial == pMaterial) {
						draw.transformations.push_back(transform.getMatrix());
						found = true;
						break;
					}
				}

				if (!found) {
					IndirectDrawData data = {};
					data.pMaterial = pMaterial;
					data.pMesh = &mesh;
					data.transformations.push_back(transform.getMatrix());
					if (pMaterial->values.opacity < 1.0f) {
						draws.push_back(data);
					}
					else {
						draws.insert(draws.begin() + lastOpaqueIndex, data);
						lastOpaqueIndex++;
					}
				}
			}
		});
	
		return std::move(draws);
	}

	void ForwardPlus::init(sa::RenderWindow* pWindow, bool setupImGui) {
		m_pWindow = pWindow;
		sa::Extent extent = m_pWindow->getCurrentExtent();

		createTextures(extent);
		createRenderPasses();
		createFramebuffers(extent);
		createPipelines(extent);

		m_linearSampler = m_renderer.createSampler(FilterMode::LINEAR);
		
		PerFrameBuffer perFrame = {};
		perFrame.projViewMatrix = sa::Matrix4x4(1);
		perFrame.viewPos = sa::Vector3(0);
		m_sceneUniformBuffer = m_renderer.createBuffer(BufferType::UNIFORM, sizeof(perFrame), &perFrame);
		
		m_lightBuffer = m_renderer.createBuffer(BufferType::STORAGE);
		m_lightBuffer << 0U;
		
		m_sceneDescriptorSet = m_renderer.allocateDescriptorSet(m_colorPipeline, SET_PER_FRAME);

		m_renderer.updateDescriptorSet(m_sceneDescriptorSet, 0, m_sceneUniformBuffer);
		m_renderer.updateDescriptorSet(m_sceneDescriptorSet, 1, m_lightBuffer);

		m_composeDescriptorSet = m_renderer.allocateDescriptorSet(m_composePipeline, 0);
		m_renderer.updateDescriptorSet(m_composeDescriptorSet, 0, m_colorTexture, m_linearSampler);
	
		m_indirectIndexedBuffer = m_renderer.createBuffer(BufferType::INDIRECT, 256);
		m_vertexBuffer = m_renderer.createBuffer(BufferType::VERTEX, 1024);
		m_indexBuffer = m_renderer.createBuffer(BufferType::INDEX, 1024);

	}

	void ForwardPlus::cleanup() {
		/*
		if (m_colorFramebuffer != NULL_RESOURCE)
			m_renderer.destroyFramebuffer(m_colorFramebuffer);

		if (m_composeFramebuffer != NULL_RESOURCE)
			m_renderer.destroyFramebuffer(m_composeFramebuffer);

		if (m_colorTexture.isValid()) m_colorTexture.destroy();
		if (m_depthTexture.isValid()) m_depthTexture.destroy();
		if (m_outputTexture.isValid()) m_outputTexture.destroy();
		*/
	}

	void ForwardPlus::beginFrameImGUI() {

	}

	void ForwardPlus::draw(Scene* pScene) {

		RenderContext context = m_pWindow->beginFrame();
		if (!context)
			return;

		context.beginRenderProgram(m_colorRenderProgram, m_colorFramebuffer, SubpassContents::DIRECT);
		context.bindPipeline(m_colorPipeline);
		context.bindDescriptorSet(m_sceneDescriptorSet, m_colorPipeline);

		
		if (pScene != nullptr) {

			std::vector<IndirectDrawData> draws = collectDrawData(pScene);


			uint32_t materialIndex = 0;

			uint32_t vertexOffset = 0;
			uint32_t indexIndex = 0;

			uint32_t totalInstanceCount = 0;
			{
				SA_PROFILE_SCOPE("Make indirect buffer");

				m_indirectIndexedBuffer.clear();
				m_vertexBuffer.clear();
				m_indexBuffer.clear();
				for (const auto& draw : draws) {
					{
						SA_PROFILE_SCOPE("Write vretecies and indices");
						m_vertexBuffer << draw.pMesh->vertices;
						m_indexBuffer << draw.pMesh->indices;
					}

					DrawIndexedIndirectCommand cmd = {};
					cmd.firstIndex = indexIndex;
					cmd.indexCount = draw.pMesh->indices.size();
					indexIndex += draw.pMesh->indices.size();
				
					cmd.firstInstance = totalInstanceCount;
					cmd.instanceCount = draw.transformations.size();
					totalInstanceCount += cmd.instanceCount;
					cmd.vertexOffset = vertexOffset;
					vertexOffset += draw.pMesh->vertices.size();
					{
						SA_PROFILE_SCOPE("Write command");

						m_indirectIndexedBuffer << cmd;
					}
				}
			}

			for (const auto& camera : pScene->getActiveCameras()) {
				SA_PROFILE_SCOPE("Draw camera");

				PerFrameBuffer perFrame;
				perFrame.projViewMatrix = camera->getProjectionMatrix() * camera->getViewMatrix();
				perFrame.viewPos = camera->getPosition();
				m_sceneUniformBuffer.write(perFrame);
				context.updateDescriptorSet(m_sceneDescriptorSet, 0, m_sceneUniformBuffer);

				if (draws.empty())
					break;
				
				//draws[0].pMaterial->bind(context, m_colorPipeline, m_linearSampler);
				AssetManager::get().getMaterial(AssetManager::get().loadDefaultMaterial())->bind(context, m_colorPipeline, m_linearSampler);

				
				context.pushConstant(m_colorPipeline, sa::ShaderStageFlagBits::VERTEX, glm::mat4(1));
				context.bindVertexBuffers(0, { m_vertexBuffer });
				context.bindIndexBuffer(m_indexBuffer);
				context.drawIndexedIndirect(m_indirectIndexedBuffer, 0, m_indirectIndexedBuffer.getElementCount<DrawIndexedIndirectCommand>(), sizeof(DrawIndexedIndirectCommand));

				/*
				for (const auto& data : draws) {
					
					context.pushConstants(m_colorPipeline, sa::ShaderStageFlagBits::VERTEX, 0, data.transformations.size() * sizeof(Matrix4x4), (void*)data.transformations.data());

					context.bindVertexBuffers(0, { data.pMesh->vertexBuffer });
					if (data.pMesh->indexBuffer.isValid()) {
						context.bindIndexBuffer(data.pMesh->indexBuffer);
						context.drawIndexed(data.pMesh->indexBuffer.getElementCount<uint32_t>(), data.transformations.size());
					}
					else {
						context.draw(data.pMesh->vertexBuffer.getElementCount<VertexUV>(), data.transformations.size());
					}
					
				}
				*/

			}

		}
		
		/*
		std::vector<std::tuple<ModelData*, Matrix4x4>> models;
		scene->forEach<comp::Transform, comp::Model>([&](const comp::Transform& transform, const comp::Model& model) {
			ModelData* pModelData = AssetManager::get().getModel(model.modelID);
			if (!pModelData)
				return;
			models.push_back({ pModelData, transform.getMatrix() });
		});

		for (const auto& camera : scene->getActiveCameras()) {


			PerFrameBuffer perFrame = {};
			perFrame.projViewMatrix = camera->getProjectionMatrix() * camera->getViewMatrix();
			perFrame.viewPos = camera->getPosition();
			m_sceneUniformBuffer.write(perFrame);

			context.updateDescriptorSet(m_sceneDescriptorSet, 0, m_sceneUniformBuffer);

			for (const auto& [pModelData, transformation] : models) {
				
				context.pushConstant(m_colorPipeline, ShaderStageFlagBits::VERTEX, transformation);

				for (const auto& mesh : pModelData->meshes) {
					Material* mat = AssetManager::get().getMaterial(mesh.materialID);
					if (mat)
						mat->bind(context, m_colorPipeline, m_linearSampler);

					context.bindVertexBuffers(0, { mesh.vertexBuffer });
					if (mesh.indexBuffer.isValid()) {
						context.bindIndexBuffer(mesh.indexBuffer);
						context.drawIndexed(mesh.indexBuffer.getElementCount<uint32_t>(), 1);
					}
					else {
						context.draw(mesh.vertexBuffer.getElementCount<VertexNormalUV>(), 1);
					}

				}
			}
		}
		*/

		context.endRenderProgram(m_colorRenderProgram);
		//context.transitionTexture(m_colorTexture, Transition::RENDER_PROGRAM_OUTPUT, Transition::FRAGMENT_SHADER_READ);

		context.beginRenderProgram(m_composeRenderProgram, m_composeFramebuffer, SubpassContents::DIRECT);
			
		context.bindPipeline(m_composePipeline);
		context.bindDescriptorSet(m_composeDescriptorSet, m_composePipeline);
		context.draw(6, 1);

		context.endRenderProgram(m_composeRenderProgram);

		m_pWindow->display();
	}

	Texture ForwardPlus::getOutputTexture() const {
		return m_outputTexture;
	}

	void ForwardPlus::updateLights(Scene* pScene) {
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
		m_renderer.updateDescriptorSet(m_sceneDescriptorSet, 1, m_lightBuffer);
	}

}