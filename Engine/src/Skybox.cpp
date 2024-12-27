#include "Assets/Skybox.h"
#include "Engine.h"
#include <Image.hpp>
#include <Shader.hpp>

namespace sa {
	void Skybox::init() {
		m_shaders[0].create(ReadSPVFile((Engine::GetShaderDirectory() / "skybox.vert.spv").generic_string().c_str()));
		m_shaders[1].create(ReadSPVFile((Engine::GetShaderDirectory() / "skybox.frag.spv").generic_string().c_str()));

		m_pipelineLayout.createFromShaders(m_shaders, 2);

		static glm::vec3 vertices[8] = {
			glm::vec3(-1, -1, -1),
			glm::vec3(1, -1, -1),
			glm::vec3(1, 1, -1),
			glm::vec3(-1, 1, -1),
			glm::vec3(-1, -1, 1),
			glm::vec3(1, -1, 1),
			glm::vec3(1, 1, 1),
			glm::vec3(-1, 1, 1)
		};
		static uint32_t indices[36] = {
			0, 1, 3, 3, 1, 2,
			1, 5, 2, 2, 5, 6,
			5, 4, 6, 6, 4, 7,
			4, 0, 7, 7, 0, 3,
			3, 2, 7, 7, 2, 6,
			4, 5, 0, 0, 5, 1
		};

		m_vertexBuffer.create(BufferType::VERTEX, sizeof(vertices), vertices);
		m_indexBuffer.create(BufferType::INDEX, sizeof(indices), indices);

		m_descriptorSet = m_pipelineLayout.allocateDescriptorSet(0);
		m_linearSampler = Renderer::Get().createSampler(FilterMode::LINEAR);
		Renderer::Get().updateDescriptorSet(m_descriptorSet, 0, m_cubemap, m_linearSampler);
	}

	bool Skybox::onLoad(JsonObject& metaData, AssetLoadFlags flags)
    {
		setCompletionCount(2);
		Image skyboxImage(getAssetPath().generic_string());
		incrementProgress();
		create(skyboxImage);
		incrementProgress();
        return true;
    }

	bool Skybox::onLoadCompiled(ByteStream& dataInStream, AssetLoadFlags flags) {
		return false;
	}

	bool Skybox::onWrite(AssetWriteFlags flags) {
		// WriteMetaFile(getMetaFilePath(), getHeader());
		return false;
	}

	bool Skybox::onCompile(ByteStream& dataOutStream, AssetWriteFlags flags) {
		return false;
	}

	bool Skybox::onUnload() {
		m_cubemap.destroy();
		m_shaders[0].destroy();
		m_shaders[1].destroy();
		m_pipelineLayout.destroy();
		if (m_pipeline != NULL_RESOURCE) {
			Renderer::Get().destroyPipeline(m_pipeline);
		}
		m_vertexBuffer.destroy();
		m_indexBuffer.destroy();
		Renderer::Get().destroySampler(m_linearSampler);
		return true;
	}

	Skybox* Skybox::clone(const std::string& name, const std::filesystem::path& assetDir) const {
		Skybox* pClone = AssetManager::Get().createAsset<Skybox>(name, assetDir);
		/*std::filesystem::path oldAssetPath = pClone->getAssetPath();
		pClone->setAssetPath(getAssetPath());
		pClone->load(AssetLoadFlagBits::FORCE_SHALLOW);
		pClone->setAssetPath(oldAssetPath);
		*/

		return pClone;
	}

    void Skybox::render(RenderContext &context, const RenderData& renderData) const {
		context.bindPipelineLayout(m_pipelineLayout);
		context.bindPipeline(m_pipeline);
		context.setViewport(renderData.viewport);
		context.setScissor(renderData.viewport);
		
		context.bindDescriptorSet(m_descriptorSet);

		context.bindVertexBuffer(0, m_vertexBuffer);
		context.bindIndexBuffer(m_indexBuffer);
		
		PerFrameBuffer data = renderData.matrices;
		data.viewMat = glm::mat4(glm::mat3(data.viewMat));
		context.pushConstants(ShaderStageFlagBits::VERTEX, 0, sizeof(glm::mat4) * 2, &data);
		
		context.drawIndexed(m_indexBuffer.getElementCount<uint32_t>(), 1);
		Engine::GetEngineStatistics().drawCalls++;
    }

    void Skybox::createPipeline(ResourceID renderProgram)
    {
        PipelineSettings settings = {};
		settings.dynamicStates.push_back(DynamicState::VIEWPORT);
		settings.dynamicStates.push_back(DynamicState::SCISSOR);
		settings.cullMode = CullModeFlagBits::NONE;
		m_pipeline = Renderer::Get().createGraphicsPipeline(m_pipelineLayout, m_shaders, 2, renderProgram, 0, { 0, 0 }, settings);
    }

    void Skybox::cleanup() {
		m_pipelineLayout.destroy();
		Renderer::Get().destroyPipeline(m_pipeline);
    }

    void Skybox::create(const Image& image) {
		m_cubemap.createCube(image, false);
		init();
	}
	
	void Skybox::create(const Image* images) {
		m_cubemap.createCube(images, false);
		init();
	}
    
	const Texture &Skybox::getTexture() const {
        return m_cubemap;
    }
}
