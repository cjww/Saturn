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
		m_initialized = true;
	}

	void Skybox::onAssetsUpdated() {
		if (m_textureAssets.size() != 1 && m_textureAssets.size() != 6)
			return;

		if (m_cubemap.isValid())
			m_cubemap.destroy();

		if (m_textureAssets.size() == 1) {
			TextureAsset* pAsset = sa::AssetManager::Get().getAsset<TextureAsset>(m_textureAssets[0]);
			if (!pAsset)
				return;
			m_cubemap.createCube(pAsset->getImage(), false);
		}
		else if (m_textureAssets.size() == 6) {
			Image images[6];
			for (int i = 0; i < m_textureAssets.size(); i++) {
				TextureAsset* pAsset = sa::AssetManager::Get().getAsset<TextureAsset>(m_textureAssets[0]);
				if (!pAsset)
					return;
				images[i] = pAsset->getImage();
			}
			m_cubemap.createCube(images, false);
		}
		if (!m_initialized) {
			init();
		}
		else {
			Renderer::Get().updateDescriptorSet(m_descriptorSet, 0, m_cubemap, m_linearSampler);
		}
	}

	bool Skybox::onLoad(JsonObject& metaData, AssetLoadFlags flags) {
		simdjson::padded_string jsonStr = simdjson::padded_string::load(getAssetPath().generic_string());
		simdjson::ondemand::parser parser;
		auto doc = parser.iterate(jsonStr);
		if (doc.error() != simdjson::error_code::SUCCESS) {
			throw std::runtime_error("Json error: " + std::string(simdjson::error_message(doc.error())));
		}
		m_textureAssets.reserve(6);
		simdjson::ondemand::array textures = doc["textures"];
		for (auto element : textures) {
			if (element.error()) {
				SA_DEBUG_LOG_WARNING("Failed to get entity from file");
				continue;
			}
			UUID textureId = element.value_unsafe().get_uint64().take_value();
			m_textureAssets.push_back(textureId);
		}	
		onAssetsUpdated();
        return true;
    }

	bool Skybox::onLoadCompiled(ByteStream& dataInStream, AssetLoadFlags flags) {
		size_t length = 0;
		dataInStream.read(&length);
		m_textureAssets.resize(length);
		dataInStream.readArray(m_textureAssets.data(), length);
		onAssetsUpdated();
		return true;
	}

	bool Skybox::onWrite(AssetWriteFlags flags) {
		WriteMetaFile(getMetaFilePath(), getHeader());
		std::ofstream file(getAssetPath());
		if (!file.good()) {
			throw std::runtime_error("Failed to open file \"" + getAssetPath().generic_string() + "\"");
		}
		Serializer serializer;
		serializer.beginObject();
		serializer.beginArray("textures");
		for (auto id : m_textureAssets)
		{
			serializer.value(id);
		}
		serializer.endArray();
		serializer.endObject();
		file << serializer.dump();
		file.close();
		return true;
	}

	bool Skybox::onCompile(ByteStream& dataOutStream, AssetWriteFlags flags) {
		dataOutStream.write(m_textureAssets.size());
		dataOutStream.writeArray(m_textureAssets.data(), m_textureAssets.size());
		return true;
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
		m_textureAssets.clear();
		m_textureAssets.shrink_to_fit();
		m_initialized = false;
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

	const std::vector<UUID>& Skybox::getTextureAssets() const {
		return m_textureAssets;
	}

	void Skybox::setTextureAssets(const std::vector<UUID>& textureAssets) {
		m_textureAssets = textureAssets;
		onAssetsUpdated();
	}
}
