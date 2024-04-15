#include "pch.h"
#include "Graphics/RenderLayers/ShadowRenderLayer.h"

#include "Engine.h"

#include "Graphics/DebugRenderer.h"

namespace sa {

	void ShadowRenderLayer::initMaterialShadowPipeline(MaterialShader* pMaterialShader, MaterialShadowPipeline& data) {
		Shader vertexShader;
		if (pMaterialShader->hasStage(ShaderStageFlagBits::VERTEX)) {
			vertexShader = *pMaterialShader->getShaderStage(ShaderStageFlagBits::VERTEX);
		}
		else {
			auto code = ReadSPVFile((Engine::GetShaderDirectory() / "ForwardPlusColorPass.vert.spv").generic_string().c_str());
			vertexShader.create(code, ShaderStageFlagBits::VERTEX);
		}


		Shader fragmentShader;
		auto code = ReadSPVFile((Engine::GetShaderDirectory() / "ShadowPass.frag.spv").generic_string().c_str());
		fragmentShader.create(code, ShaderStageFlagBits::FRAGMENT);

		Shader shaders[] = { vertexShader, fragmentShader };
		data.pipelineLayout.createFromShaders(shaders, 2);

		PipelineSettings settings = {};
		settings.dynamicStates.push_back(DynamicState::VIEWPORT);
		settings.dynamicStates.push_back(DynamicState::SCISSOR);
		settings.dynamicStates.push_back(DynamicState::DEPTH_BIAS);
		settings.dynamicStates.push_back(DynamicState::DEPTH_BIAS_ENABLE);
		settings.dynamicStates.push_back(DynamicState::CULL_MODE);

		data.pipeline = Renderer::Get().createGraphicsPipeline(data.pipelineLayout, shaders, 2, m_depthRenderProgram, 0, { 0, 0 }, settings);
		
		data.isInitialized = true;
	}

	void ShadowRenderLayer::cleanupRenderData(ShadowRenderData& data) {
		if (data.depthTexture.isValid())
			data.depthTexture.destroy();

		for (uint32_t i = 0; i < data.depthTextureLayers.size(); i++) {
			if(data.depthTextureLayers[i].isValidView())
				data.depthTextureLayers[i].destroy();

			if (data.depthFramebuffers[i] != NULL_RESOURCE) {
				m_renderer.destroyFramebuffer(data.depthFramebuffers[i]);
				data.depthFramebuffers[i] = NULL_RESOURCE;
			}
		}
		data.isInitialized = false;
	}

	void ShadowRenderLayer::initializeRenderData(ShadowRenderData& data, LightType lightType) {
		ShadowPreferences& prefs = getPreferences();
		
		uint32_t count = ShadowPreferences::MaxCascadeCount;
		switch (lightType) {
		case LightType::DIRECTIONAL:
			
			data.depthTexture.create2D(
				TextureUsageFlagBits::DEPTH_ATTACHMENT | TextureUsageFlagBits::SAMPLED,
				{ prefs.directionalMapResolution, prefs.directionalMapResolution },
				m_depthFormat,
				1,
				count,
				1
			);
			break;
		case LightType::POINT:
			data.depthTexture.createCube(
				TextureUsageFlagBits::DEPTH_ATTACHMENT | TextureUsageFlagBits::SAMPLED,
				{ prefs.omniMapResolution, prefs.omniMapResolution },
				m_depthFormat,
				1,
				1
			);
			break;
		default:
			data.depthTexture.create2D(
				TextureUsageFlagBits::DEPTH_ATTACHMENT | TextureUsageFlagBits::SAMPLED,
				{ prefs.directionalMapResolution, prefs.directionalMapResolution },
				m_depthFormat,
				1,
				count,
				1
			);
			break;
		}
		data.lightType = lightType;
		data.depthTexture.createArrayLayerTextures(&count, data.depthTextureLayers.data());

		for (uint32_t i = 0; i < count; i++) {
			data.depthFramebuffers[i] = m_renderer.createFramebuffer(m_depthRenderProgram, { data.depthTextureLayers[i] }, data.depthTexture.getExtent());
		}
		data.isInitialized = true;
	}

	void ShadowRenderLayer::renderMaterialCollection(RenderContext& context, MaterialShaderCollection& collection, ShadowData& data, const ShadowRenderData& renderData, uint32_t layer) {
		MaterialShader* pMaterialShader = collection.getMaterialShader();
		if (!pMaterialShader)
			return;
		MaterialShadowPipeline& materialPipeline = m_materialShaderPipelines[pMaterialShader->getID()];
		if (!materialPipeline.isInitialized)
			initMaterialShadowPipeline(pMaterialShader, materialPipeline);


		const auto& prefs = getPreferences();

		context.beginRenderProgram(m_depthRenderProgram, renderData.depthFramebuffers[layer], SubpassContents::DIRECT);
		
		context.bindPipelineLayout(materialPipeline.pipelineLayout);
		context.bindPipeline(materialPipeline.pipeline);

		context.bindDescriptorSet(collection.getSceneDescriptorSetDepthPass());

		context.bindVertexBuffers(0, &collection.getVertexBuffer(), 1);
		context.bindIndexBuffer(collection.getIndexBuffer());

		Rect viewport = {};
		viewport.extent = Renderer::Get().getFramebufferExtent(renderData.depthFramebuffers[layer]);
		viewport.offset = { 0, 0 };

		context.setViewport(viewport);
		context.setScissor(viewport);

		context.setDepthBiasEnable(true);
		context.setDepthBias(prefs.depthBiasConstant, 0.0f, prefs.depthBiasSlope);

		context.setCullMode(sa::CullModeFlagBits::FRONT);

		PerFrameBuffer perFrame = {};
		perFrame.viewMat = data.lightViewMatrices[layer];
		perFrame.projMat = data.lightProjMatrices[layer];
		perFrame.viewPos = data.lightPosition;

		if (collection.getDrawCommandBuffer().getElementCount<DrawIndexedIndirectCommand>() > 0) {
			context.pushConstant(ShaderStageFlagBits::VERTEX | ShaderStageFlagBits::FRAGMENT, perFrame);
			uint32_t linearizeDepth = data.lightType == LightType::POINT ? 1u : 0u;
			context.pushConstant(ShaderStageFlagBits::FRAGMENT, linearizeDepth, sizeof(perFrame));
			context.drawIndexedIndirect(collection.getDrawCommandBuffer(), 0, collection.getDrawCommandBuffer().getElementCount<DrawIndexedIndirectCommand>(), sizeof(DrawIndexedIndirectCommand));
		}
		context.endRenderProgram(m_depthRenderProgram);
	}

	void ShadowRenderLayer::updateCascadeSplits(float near, float far) {
		const auto& prefs = getPreferences();

		const int cascadeCount = prefs.cascadeCount;
		const float cascadeSplitLambda = prefs.cascadeSplitLambda;

		float clipRange = far - near;
		float clipRatio = far / near;

		float cascadeSplits[ShadowPreferences::MaxCascadeCount];
		for (uint32_t i = 0; i < cascadeCount; i++) {
			// Based on method presented in https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
			float p = (i + 1) / static_cast<float>(cascadeCount);
			float log = near * std::pow(clipRatio, p);
			float uniform = near + clipRange * p;
			float d = cascadeSplitLambda * (log - uniform) + uniform;

			m_cascadeSplitsNormalized[i] = (d - near) / clipRange; // [0, 1] range
			m_cascadeSplits[i] = -(near + m_cascadeSplitsNormalized[i] * clipRange); // negative [near, far] range
		}
	}

	void ShadowRenderLayer::calculateCascadeMatrices(const SceneCamera& sceneCamera, ShadowData& data) {
		const auto& prefs = getPreferences();
		const int cascadeCount = prefs.cascadeCount;

		static glm::vec3 frustumPoints[8];
		sceneCamera.calculateFrustumBoundsWorldSpace(frustumPoints);

		SceneCamera camera;
		camera.setProjectionMode(ProjectionMode::eOrthographic);
		float lastSplitDistance = 0.0f;
		for (uint32_t i = 0; i < cascadeCount; i++) {
			const float splitDistance = m_cascadeSplitsNormalized[i];

			glm::vec3 cascadePoints[8];
			std::copy(std::begin(frustumPoints), std::end(frustumPoints), std::begin(cascadePoints));
			// calculate cascade corners
			for (uint32_t j = 0; j < 4; j++) {
				glm::vec3 dist = cascadePoints[j + 4] - cascadePoints[j];
				cascadePoints[j + 4] = cascadePoints[j] + (dist * splitDistance);
				cascadePoints[j] = cascadePoints[j] + (dist * lastSplitDistance);
			}

			// Get center
			glm::vec3 center(0.f);
			for (uint32_t j = 0; j < 8; j++) {
				center += cascadePoints[j];
			}
			center /= 8.0f;

			// calculate encapsulation
			float radius = 0.0f;
			for (uint32_t j = 0; j < 8; j++) {
				float distance = glm::length(cascadePoints[j] - center);
				radius = glm::max(radius, distance);
			}
			radius = std::ceil(radius * 16.0f) / 16.0f;

			const glm::vec3 maxExtents(radius);
			const glm::vec3 minExtents(-maxExtents);

			const float distance = maxExtents.z * 10.f;
			camera.setPosition(center - glm::vec3(data.lightDirection) * distance);
			camera.lookAt(center);
			camera.setNear(0.0f);
			camera.setFar(distance - minExtents.z);
			camera.setOrthoBounds(sa::Bounds{
				.left = minExtents.x,
				.right = maxExtents.x,
				.top = maxExtents.y,
				.bottom = minExtents.y
				});

			data.lightViewMatrices[i] = camera.getViewMatrix();
			data.lightProjMatrices[i] = camera.getProjectionMatrix();
			lastSplitDistance = splitDistance;
		}
	}


	void ShadowRenderLayer::renderCascadedShadowMaps(RenderContext& context, const SceneCamera& sceneCamera, ShadowData& data, const ShadowRenderData& renderData, SceneCollection& sceneCollection) {
		const auto& prefs = getPreferences();

		const int cascadeCount = prefs.cascadeCount;
		
		for (auto& collection : sceneCollection) {
			if (!collection.readyDescriptorSets(context)) {
				continue;
			}

			if (!collection.arePipelinesReady()) {
				continue;
			}
			for (uint32_t i = 0; i < cascadeCount; i++) {
				renderMaterialCollection(context, collection, data, renderData, i);
			}
		}
	}

	void ShadowRenderLayer::createSampler() {
		const auto& prefs = getPreferences();
		SamplerInfo info = {};
		info.addressModeU = SamplerAddressMode::CLAMP_TO_BORDER;
		info.addressModeV = SamplerAddressMode::CLAMP_TO_BORDER;
		info.addressModeW = SamplerAddressMode::CLAMP_TO_BORDER;
		info.borderColor = sa::BorderColor::FLOAT_OPAQUE_WHITE;
		info.magFilter =  prefs.softShadows ? FilterMode::LINEAR : FilterMode::NEAREST;
		info.minFilter = prefs.softShadows ? FilterMode::LINEAR : FilterMode::NEAREST;
		info.compareEnable = true;
		info.compareOp = CompareOp::GREATER;

		m_shadowSampler = m_renderer.createSampler(info);
	}

	void ShadowRenderLayer::renderCubeMapShadows(RenderContext& context, ShadowData& data, const ShadowRenderData& renderData, SceneCollection& sceneCollection) {
		static const std::array<glm::vec3, 6> faces = {
			glm::vec3(-1, 0, 0),	// +X
			glm::vec3(1, 0, 0),		// -X
			glm::vec3(0, 1, 0),		// +Y
			glm::vec3(0, -1, 0),	// -Y
			glm::vec3(0, 0, 1),		// +Z
			glm::vec3(0, 0, -1),	// -Z
		};
		static const std::array<glm::vec3, 6> ups = {
			glm::vec3(0, -1, 0),
			glm::vec3(0, -1, 0),
			glm::vec3(0, 0, 1),
			glm::vec3(0, 0, -1),
			glm::vec3(0, -1, 0),
			glm::vec3(0, -1, 0),
		};

		SceneCamera camera;
		camera.setProjectionMode(ProjectionMode::ePerspective);
		camera.setFOVDegrees(90.f);
		camera.setFar(data.lightPosition.w);
		camera.setNear(0.001f);
		camera.setPosition(data.lightPosition);
		
		glm::mat4 projMat = camera.getProjectionMatrix();

		for (uint32_t i = 0; i < faces.size(); i++) {
			camera.setUp(ups[i]);
			camera.setForward(faces[i]);

			data.lightProjMatrices[i] = projMat;
			data.lightViewMatrices[i] = camera.getViewMatrix();

			for (auto& collection : sceneCollection) {
				if (!collection.readyDescriptorSets(context)) {
					continue;
				}

				if (!collection.arePipelinesReady()) {
					continue;
				}
				renderMaterialCollection(context, collection, data, renderData, i);
			}

		}

	}

	void ShadowRenderLayer::renderSingleDirectedShadow(RenderContext& context, ShadowData& data, const ShadowRenderData& renderData, SceneCollection& sceneCollection) {
		
		SceneCamera camera;
		camera.setProjectionMode(ProjectionMode::ePerspective);
		camera.setAspectRatio(1.f);
		camera.setFar(data.lightPosition.w);
		camera.setNear(0.1f);
		camera.setFOVRadians(data.lightDirection.w * 2);

		data.lightProjMatrices[0] = camera.getProjectionMatrix();

		camera.setPosition(data.lightPosition);
		camera.setForward(data.lightDirection);
		
		data.lightViewMatrices[0] = camera.getViewMatrix();

		for (auto& collection : sceneCollection) {
			if (!collection.readyDescriptorSets(context)) {
				continue;
			}

			if (!collection.arePipelinesReady()) {
				continue;
			}
			renderMaterialCollection(context, collection, data, renderData, 0);
		}
	}


	void ShadowRenderLayer::renderShadowMap(RenderContext& context, const SceneCamera& sceneCamera, ShadowData& data, const ShadowRenderData& renderData, SceneCollection& sceneCollection) {
		
		switch(data.lightType) {
		case LightType::DIRECTIONAL:
			calculateCascadeMatrices(sceneCamera, data);
			renderCascadedShadowMaps(context, sceneCamera, data, renderData, sceneCollection);

			break;
		case LightType::POINT:
			renderCubeMapShadows(context, data, renderData, sceneCollection);

			break;
		case LightType::SPOT:
			renderSingleDirectedShadow(context, data, renderData, sceneCollection);
			break;
		default:
			break;
		}
		
	}

	void ShadowRenderLayer::init() {
		//m_depthFormat = sa::Format::D16_UNORM;
		m_depthFormat = m_renderer.getDefaultDepthFormat();
		
		if (m_depthRenderProgram != NULL_RESOURCE) {
			m_renderer.destroyRenderProgram(m_depthRenderProgram);
			m_depthRenderProgram = NULL_RESOURCE;
		}

		m_depthRenderProgram = m_renderer.createRenderProgram()
			.addDepthAttachment(AttachmentFlagBits::eClear | AttachmentFlagBits::eSampled | AttachmentFlagBits::eStore, m_depthFormat)
			.beginSubpass()
			.addAttachmentReference(0, SubpassAttachmentUsage::DepthTarget)
			.endSubpass()
			.end();
	
		createSampler();

		SceneCamera temp;
		updateCascadeSplits(temp.getNear(), temp.getFar()); // update cascades based on default camera near and far

		const auto& prefs = getPreferences();
		ShadowPreferencesShaderData data = {
			.cascadeCount = prefs.cascadeCount,
			.softShadows = prefs.softShadows,
			.showDebugCascades = prefs.showCascades
		}; 
		std::copy(m_cascadeSplits.begin(), m_cascadeSplits.end(), std::begin(data.cascadeSplits));
		m_preferencesBuffer.create(BufferType::UNIFORM, sizeof(data), &data);

	}

	void ShadowRenderLayer::cleanup() {
		if (m_depthRenderProgram != NULL_RESOURCE) {
			m_renderer.destroyRenderProgram(m_depthRenderProgram);
			m_depthRenderProgram = NULL_RESOURCE;
		}
	}
	
	void ShadowRenderLayer::onRenderTargetResize(UUID renderTargetID, Extent oldExtent, Extent newExtent) {

	}

	void ShadowRenderLayer::onPreferencesUpdated() {
		
		SceneCamera temp;
		updateCascadeSplits(temp.getNear(), temp.getFar()); // update cascades based on default camera near and far
		
		const auto& prefs = getPreferences();
		
		ShadowPreferencesShaderData data = {};
		data.cascadeCount = prefs.cascadeCount;
		data.softShadows = prefs.softShadows;
		data.showDebugCascades = prefs.showCascades;
		std::copy(m_cascadeSplits.begin(), m_cascadeSplits.end(), std::begin(data.cascadeSplits));
		m_preferencesBuffer.write(data);

		if (m_shadowSampler != NULL_RESOURCE)
			m_renderer.destroySampler(m_shadowSampler);
		createSampler();

		forEachRenderData([](ShadowRenderData& renderData) {
			renderData.isInitialized = false;
		});
	}

	void ShadowRenderLayer::renderShadowMap(
		RenderContext& context,
		const SceneCamera& sceneCamera,
		ShadowData& data,
		ShadowRenderData& renderData,
		SceneCollection& sceneCollection,
		uint32_t layerCount,
		uint32_t index)
	{

		/*
		if (sceneCollection.getShadowTextureCount() >= MAX_SHADOW_TEXTURE_COUNT) {
			SA_DEBUG_LOG_WARNING("Exceeding maximum limit of ", MAX_SHADOW_TEXTURE_COUNT, " shadowmaps");
			return;
		}
		*/

		if (renderData.lightType != data.lightType)
			renderData.isInitialized = false;
		
		if (!renderData.isInitialized) {
			cleanupRenderData(renderData);
			initializeRenderData(renderData, data.lightType);
		}

		renderShadowMap(context, sceneCamera, data, renderData, sceneCollection);

		ShadowShaderData shaderData = {};
		for (uint32_t i = 0; i < layerCount; i++) {
			shaderData.lightMat[i] = data.lightProjMatrices[i] * data.lightViewMatrices[i];
		}
		sceneCollection.insertShaderData(shaderData, renderData.depthTexture, index);
	}

	bool ShadowRenderLayer::preRender(RenderContext& context, SceneCollection& sceneCollection) {
		SA_PROFILE_FUNCTION();
		for (auto it = sceneCollection.iterateShadowsBegin(); it != sceneCollection.iterateShadowsEnd(); it++) {
			sa::ShadowData& data = *it;
			if (data.lightType == LightType::DIRECTIONAL)
				continue;

			ShadowRenderData& renderData = getRenderTargetData(static_cast<uint64_t>(data.entityID));
			uint32_t index = it - sceneCollection.iterateShadowsBegin();

			switch (data.lightType) {
			case LightType::POINT:
				renderShadowMap(context, 
					SceneCamera(),
					data,
					renderData,
					sceneCollection,
					6,
					index);
				break;
			case LightType::SPOT:
				renderShadowMap(context,
					SceneCamera(),
					data,
					renderData,
					sceneCollection,
					1,
					index);
				break;
			default:
				break;
			}

		}
		return true;
	}
	
	bool ShadowRenderLayer::render(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget, SceneCollection& sceneCollection) {
		SA_PROFILE_FUNCTION();
		const auto& prefs = getPreferences();
		for (auto it = sceneCollection.iterateShadowsBegin(); it != sceneCollection.iterateShadowsEnd(); it++) {
			sa::ShadowData& data = *it;
			if (data.lightType != LightType::DIRECTIONAL)
				continue;

			ShadowRenderData& renderData = getRenderTargetData(pRenderTarget->getID() ^ static_cast<uint32_t>(data.entityID));
			renderShadowMap(context, 
				*pCamera,
				data,
				renderData,
				sceneCollection,
				prefs.cascadeCount,
				it - sceneCollection.iterateShadowsBegin());

		}
		return true;
	}

	bool ShadowRenderLayer::postRender(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget, SceneCollection& sceneCollection) {
		
		return true;
	}

	const ResourceID ShadowRenderLayer::getShadowSampler() const {
		return m_shadowSampler;
	}
	
	const Buffer& ShadowRenderLayer::getPreferencesBuffer() const {
		return m_preferencesBuffer;
	}
}
