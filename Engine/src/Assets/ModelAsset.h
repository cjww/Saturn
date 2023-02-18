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

		bool loadAssimpModel(const std::filesystem::path& path);
		virtual bool unload() override;

	public:
		using IAsset::IAsset;
		static AssetTypeID type() { return s_typeID; }

		static bool isExtensionSupported(const std::string& extension);

		//Data
		ModelData data;

		virtual bool create(const std::string& name) override;
		virtual bool importFromFile(const std::filesystem::path& path) override;

		virtual bool load() override;
		virtual bool write() override;



	};

}