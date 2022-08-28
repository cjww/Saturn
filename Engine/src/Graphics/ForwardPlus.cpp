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

	}

	void ForwardPlus::createFramebuffers(sa::Extent extent) {
		if (m_colorFramebuffer != NULL_RESOURCE)
			m_renderer.destroyFramebuffer(m_colorFramebuffer);
		m_colorFramebuffer = m_renderer.createFramebuffer(m_colorRenderProgram, { m_colorTexture, m_depthTexture });
	}

	void ForwardPlus::init(sa::Extent extent, bool setupImGui) {
		sa::Extent extent = m_pWindow->getCurrentExtent();

		createTextures(extent);
		createRenderPasses();
		createFramebuffers(extent);


		m_colorPipeline = m_renderer.createGraphicsPipeline(m_colorRenderProgram, 0, extent,
			"../Engine/shaders/ForwardPlusColorPass.vert.spv", "../Engine/shaders/ForwardPlusColorPass.frag.spv");

		PerFrameBuffer perFrame = {};
		perFrame.projViewMatrix = sa::Matrix4x4(1);
		perFrame.viewPos = sa::Vector3(0);
		
		m_sceneDescriptorSet = m_renderer.allocateDescriptorSet(m_colorPipeline, SET_PER_FRAME);
		m_sceneUniformBuffer = m_renderer.createBuffer(BufferType::UNIFORM, sizeof(perFrame), &perFrame);

		m_linearSampler = m_renderer.createSampler(FilterMode::LINEAR);

		m_lightBuffer = m_renderer.createBuffer(BufferType::STORAGE);
		m_lightBuffer << 0U;

		m_renderer.updateDescriptorSet(m_sceneDescriptorSet, 1, m_lightBuffer);
	}

	void ForwardPlus::cleanup() {
		if (m_colorFramebuffer != NULL_RESOURCE)
			m_renderer.destroyFramebuffer(m_colorFramebuffer);

		if (m_colorTexture.isValid()) m_colorTexture.destroy();
		if (m_depthTexture.isValid()) m_depthTexture.destroy();
		if (m_outputTexture.isValid()) m_outputTexture.destroy();
	}

	void ForwardPlus::beginFrameImGUI() {

	}

	void ForwardPlus::draw(const Scene* scene) {

		RenderContext context = m_pWindow->beginFrame();
		if (!context)
			return;

		for (const auto& camera : scene->getActiveCameras()) {


			PerFrameBuffer perFrame = {};
			perFrame.projViewMatrix = camera->getProjectionMatrix() * camera->getViewMatrix();
			perFrame.viewPos = camera->getPosition();
			m_sceneUniformBuffer.write(perFrame);
			
			context.updateDescriptorSet(m_sceneDescriptorSet, 0, m_sceneUniformBuffer);

			context.beginRenderProgram(m_colorRenderProgram, m_colorFramebuffer, SubpassContents::DIRECT);

			context.bindPipeline(m_colorPipeline);
			context.bindDescriptorSet(m_sceneDescriptorSet, m_colorPipeline);

			scene->forEach<comp::Transform, comp::Model>([&](const comp::Transform& transform, const comp::Model& model) {
				ModelData* pModelData = AssetManager::get().getModel(model.modelID);
				if (!pModelData)
					return;

				context.pushConstant(m_colorPipeline, ShaderStageFlagBits::VERTEX, transform.getMatrix());

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
			});

			context.endRenderProgram(m_colorRenderProgram);

			m_pWindow->display();
		}
	}

	Texture ForwardPlus::getOutputTexture() const {
		return m_outputTexture;
	}

	void ForwardPlus::updateLights(Scene* pScene) {

	}

}