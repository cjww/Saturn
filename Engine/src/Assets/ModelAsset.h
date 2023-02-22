#pragma once

#include "IAsset.h"
#include "AssetManager.h"

namespace sa {

	struct Mesh {
		std::vector<VertexNormalUV> vertices;
		std::vector<uint32_t> indices;

		UUID materialID;
	};

	struct ModelData {
		std::vector<Mesh> meshes;
	};

	class ModelAsset : public IAsset {
	private:
		friend class AssetManager;
		inline static AssetTypeID s_typeID;

		void processNode(const void* scene, const void* node);
		bool loadAssimpModel(const std::filesystem::path& path);


	public:
		using IAsset::IAsset;
		static AssetTypeID type() { return s_typeID; }

		static bool isExtensionSupported(const std::string& extension);

		//Data
		ModelData data;

		virtual bool onImport(const std::filesystem::path& path) override;


		virtual bool onLoad(std::ifstream& file, AssetLoadFlags flags) override;
		virtual bool onWrite(std::ofstream& file, AssetWriteFlags flags) override;
		virtual bool onUnload() override;




	};

}