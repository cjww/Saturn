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
		m_vertexBuffer.clear();
		m_indexBuffer.clear();
		m_indirectIndexedBuffer.clear();
		m_objectBuffer.clear();
		m_materialBuffer.clear();

		m_draws.clear();
		
		m_vertices.clear();
		m_indices.clear();
		m_indirectCommands.clear();
		m_objects.clear();

		uint32_t objectID = 0U;

		
		pScene->forEach<comp::Transform, comp::Model>([&](const comp::Transform& transform, const comp::Model& modelComp) {
			if (modelComp.modelID == NULL_RESOURCE)
				return; // does not have to be drawn

			// Store transformation
			ObjectBuffer object = {};
			object.worldMat = transform.getMatrix();
			m_objectBuffer << object;
			//m_objects.push_back(object);
			// Get Model
			ModelData* pModel = sa::AssetManager::get().getModel(modelComp.modelID);
			
			// For each mesh
			for (auto& mesh : pModel->meshes) {
				Material* pMaterial = AssetManager::get().getMaterial(mesh.materialID);
				
				// Look if mesh is already in the vertexbuffer
				bool found = false;
				uint32_t i = 0;
				for (auto& draw : m_draws) {
					if (draw.pMesh == &mesh && draw.pMaterial == pMaterial) {
						// If it is, add an instance	
						auto& cmd = m_indirectIndexedBuffer.at<DrawIndexedIndirectCommand>(i);
						//auto& cmd = m_indirectCommands.at(i);
						cmd.instanceCount++;

						found = true;
						break;
					}
					i++;
				}

				if (!found) {
					// If not, store data
					IndirectDrawData data = {};
					data.pMaterial = pMaterial;
					data.pMesh = &mesh;
					m_draws.push_back(data);

					// Push mesh into buffers
					uint32_t vertexOffset = m_vertexBuffer.getElementCount<VertexNormalUV>();
					m_vertexBuffer << mesh.vertices;
					
					//uint32_t vertexOffset = m_vertices.size();
					//m_vertices.insert(m_vertices.end(), mesh.vertices.begin(), mesh.vertices.end());

					uint32_t firstIndex = m_indexBuffer.getElementCount<uint32_t>();
					m_indexBuffer << mesh.indices;

					//uint32_t firstIndex = m_indices.size();
					//m_indices.insert(m_indices.end(), mesh.indices.begin(), mesh.indices.end());

					// Create a draw command for this mesh
					DrawIndexedIndirectCommand cmd = {};
					cmd.firstIndex = firstIndex;
					cmd.indexCount = mesh.indices.size();
					cmd.firstInstance = objectID;
					cmd.instanceCount = 1;
					cmd.vertexOffset = vertexOffset;
					m_indirectIndexedBuffer << cmd;
					//m_indirectCommands.push_back(cmd);
					
				}
			}
			objectID++;
		});

		//m_vertexBuffer.write(m_vertices);
		//m_indexBuffer.write(m_indices);
		//m_objectBuffer.write(m_objects);
		//m_indirectIndexedBuffer.write(m_indirectCommands);

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