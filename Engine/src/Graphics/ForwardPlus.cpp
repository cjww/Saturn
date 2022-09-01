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

	void ForwardPlus::collectMeshes(Scene* pScene) {
		SA_PROFILE_FUNCTION();
		m_objectBuffer.clear();
		m_vertexBuffer.clear();
		m_indexBuffer.clear();
		m_indirectIndexedBuffer.clear();

		m_materialBuffer.clear(); // TODO add material support

		m_meshes.clear();
		m_uniqueMeshes.clear();
		m_uniqueMeshCount.clear();
		m_uniqueMeshFirstObjectIndex.clear();


		m_objectBuffer.reserve(pScene->view<comp::Transform, comp::Model>().size_hint() * sizeof(ObjectBuffer), IGNORE_CONTENT);
		
		uint32_t objectIndex = 0U;
		uint32_t vertexCount = 0;
		uint32_t indexCount = 0;
		pScene->view<comp::Transform, comp::Model>().each([&](const comp::Transform& transform, const comp::Model& modelComp) {
			if (modelComp.modelID == NULL_RESOURCE)
				return; // does not have to be drawn

			ObjectBuffer object = {};
			object.worldMat = transform.getMatrix();
			m_objectBuffer << object;
			
			ModelData* pModel = sa::AssetManager::get().getModel(modelComp.modelID);
			m_meshes.reserve(m_meshes.size() + pModel->meshes.size());
			for (auto& mesh : pModel->meshes) {
				m_meshes.push_back({ &mesh, objectIndex });
				auto it = std::find(m_uniqueMeshes.begin(), m_uniqueMeshes.end(), &mesh);
				if (it == m_uniqueMeshes.end()) {
					m_uniqueMeshes.push_back(&mesh);
					vertexCount += mesh.vertices.size();
					indexCount += mesh.indices.size();
					m_uniqueMeshCount.push_back(1);
					m_uniqueMeshFirstObjectIndex.push_back(objectIndex);
				}
				else {
					m_uniqueMeshCount[std::distance(m_uniqueMeshes.begin(), it)]++;
				}
			}
			objectIndex++;
		});

		m_vertexBuffer.reserve(vertexCount * sizeof(VertexNormalUV), IGNORE_CONTENT);
		m_indexBuffer.reserve(indexCount * sizeof(uint32_t), IGNORE_CONTENT);
		m_indirectIndexedBuffer.reserve(m_uniqueMeshes.size() * sizeof(DrawIndexedIndirectCommand), IGNORE_CONTENT);

		for (size_t i = 0; i < m_uniqueMeshes.size(); i++) {
			
			// Push mesh into buffers
			uint32_t vertexOffset = m_vertexBuffer.getElementCount<VertexNormalUV>();
			m_vertexBuffer << m_uniqueMeshes[i]->vertices;

			uint32_t firstIndex = m_indexBuffer.getElementCount<uint32_t>();
			m_indexBuffer << m_uniqueMeshes[i]->indices;

			// Create a draw command for this mesh
			DrawIndexedIndirectCommand cmd = {};
			cmd.firstIndex = firstIndex;
			cmd.indexCount = m_uniqueMeshes[i]->indices.size();
			cmd.firstInstance = m_uniqueMeshFirstObjectIndex[i];
			cmd.instanceCount = m_uniqueMeshCount[i];
			cmd.vertexOffset = vertexOffset;
			m_indirectIndexedBuffer << cmd;

		}
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
		
		unsigned int lightCount = 0;
		m_lightBuffer = m_renderer.createDynamicBuffer(BufferType::STORAGE, sizeof(lightCount), &lightCount);
		
		m_sceneDescriptorSet = m_renderer.allocateDescriptorSet(m_colorPipeline, SET_PER_FRAME);


		m_composeDescriptorSet = m_renderer.allocateDescriptorSet(m_composePipeline, 0);
		m_renderer.updateDescriptorSet(m_composeDescriptorSet, 0, m_colorTexture, m_linearSampler);
	
		m_indirectIndexedBuffer = m_renderer.createDynamicBuffer(BufferType::INDIRECT, 256);
		m_vertexBuffer = m_renderer.createDynamicBuffer(BufferType::VERTEX, 100000);
		m_indexBuffer = m_renderer.createDynamicBuffer(BufferType::INDEX, 100000);
		m_objectBuffer = m_renderer.createDynamicBuffer(BufferType::STORAGE);
		
		m_materialBuffer = m_renderer.createDynamicBuffer(BufferType::STORAGE);

		m_renderer.updateDescriptorSet(m_sceneDescriptorSet, 0, m_objectBuffer);
		m_renderer.updateDescriptorSet(m_sceneDescriptorSet, 1, m_sceneUniformBuffer);
		m_renderer.updateDescriptorSet(m_sceneDescriptorSet, 2, m_lightBuffer);


		//DEBUG
		m_renderer.setClearColor(m_colorRenderProgram, Color{ 0.3f, 0.3f, 0.3f });

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

		auto stats = m_renderer.getGPUMemoryUsage();
		
		if (pScene != nullptr) {

			collectMeshes(pScene);

			context.updateDescriptorSet(m_sceneDescriptorSet, 0, m_objectBuffer);
			context.bindVertexBuffers(0, { m_vertexBuffer });
			context.bindIndexBuffer(m_indexBuffer);

			if (m_indirectIndexedBuffer.getElementCount<DrawIndexedIndirectCommand>() > 0) {

				for (const auto& camera : pScene->getActiveCameras()) {
					SA_PROFILE_SCOPE("Draw camera");

					PerFrameBuffer perFrame;
					perFrame.projViewMatrix = camera->getProjectionMatrix() * camera->getViewMatrix();
					perFrame.viewPos = camera->getPosition();
					m_sceneUniformBuffer.write(perFrame);
					context.updateDescriptorSet(m_sceneDescriptorSet, 1, m_sceneUniformBuffer);


					AssetManager::get().getMaterial(AssetManager::get().loadDefaultMaterial())->bind(context, m_colorPipeline, m_linearSampler);
					//draws[0].pMaterial->bind(context, m_colorPipeline, m_linearSampler);

				
					//context.pushConstant(m_colorPipeline, sa::ShaderStageFlagBits::VERTEX, glm::mat4(1));
					context.drawIndexedIndirect(m_indirectIndexedBuffer, 0, m_indirectIndexedBuffer.getElementCount<DrawIndexedIndirectCommand>(), sizeof(DrawIndexedIndirectCommand));

				}
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