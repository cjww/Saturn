#pragma once

#include "Asset.h"
#include "AssetHolder.h"
#include "Graphics/Material.h"
#include "Vertex.h"

namespace sa {

	struct Mesh {
		std::vector<VertexNormalUV> vertices;
		std::vector<uint32_t> indices;

		AssetHolder<Material> material;
	};

	struct ModelData {
		std::vector<Mesh> meshes;
	};

	class ModelAsset : public Asset {
	private:
		void processNode(const void* scene, const void* node, std::vector<uint32_t>& materialIndices);
		bool loadAssimpModel(const std::filesystem::path& path);
	public:
		//Data
		ModelData data;


		using Asset::Asset;

		static bool IsExtensionSupported(const std::string& extension);

		virtual bool onImport(const std::filesystem::path& path) override;

		virtual bool onLoad(JsonObject& metaData, AssetLoadFlags flags) override;
		virtual bool onLoadCompiled(ByteStream& dataInStream, AssetLoadFlags flags) override;

		virtual bool onWrite(AssetWriteFlags flags) override;
		virtual bool onCompile(ByteStream& dataOutStream, AssetWriteFlags flags) override;

		virtual bool onUnload() override;

		ModelAsset* clone(const std::string& name, const std::filesystem::path& assetDir = "") const override;



	};

}