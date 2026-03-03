#pragma once
#include "Asset.h"
#include "structs.h"
#include <Resources\Texture.hpp>
#include <PipelineLayout.hpp>
#include <Shader.hpp>
#include <Resources/Buffer.hpp>
#include <RenderContext.hpp>

namespace sa {
	class Skybox : public Asset {
	private:
		Texture m_cubemap;
		Shader m_shaders[2];
		PipelineLayout m_pipelineLayout;
		ResourceID m_pipeline = NULL_RESOURCE;

		Buffer m_vertexBuffer;
		Buffer m_indexBuffer;
		ResourceID m_descriptorSet = NULL_RESOURCE;
		
		ResourceID m_linearSampler = NULL_RESOURCE;
		bool m_initialized = false;

		std::vector<UUID> m_textureAssets;

		void init();
		void onAssetsUpdated();
	protected:
		// [DO NOT USE] Called by load. Do not call directly
		virtual bool onLoad(JsonObject& metaData, AssetLoadFlags flags) override;
		virtual bool onLoadCompiled(ByteStream& dataInStream, AssetLoadFlags flags) override;

		// [DO NOT USE] Called by write. Do not call directly
		virtual bool onWrite(AssetWriteFlags flags) override;
		virtual bool onCompile(ByteStream& dataOutStream, AssetWriteFlags flags) override;
		
		virtual bool onUnload() override;

	public:
		using Asset::Asset;

		void createPipeline(ResourceID renderProgram);
		void cleanup();

		void create(const Image& image);
		void create(const Image* images);

		const Texture& getTexture() const;

		const std::vector<UUID>& getTextureAssets() const;
		void setTextureAssets(const std::vector<UUID>& textureAssets);

		virtual Skybox* clone(const std::string& name, const std::filesystem::path& assetDir = "") const override;

    	void render(RenderContext &context, const RenderData& renderData) const;
	};
}

