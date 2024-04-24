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
		using Asset::Asset;


		static bool IsExtensionSupported(const std::string& extension);

		//Data
		ModelData data;

		virtual bool onImport(const std::filesystem::path& path) override;


		virtual bool onLoad(std::ifstream& file, AssetLoadFlags flags) override;
		virtual bool onLoadCompiled(ByteStream& byteStream, AssetLoadFlags flags) override;

		virtual bool onWrite(std::ofstream& file, AssetWriteFlags flags) override;
		virtual bool onUnload() override;




	};

}