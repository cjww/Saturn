#pragma once

#include "IAsset.h"
#include "AssetManager.h"

namespace sa {

	class ModelAsset : public IAsset {
	private:
		friend class AssetManager;
		inline static AssetTypeID s_typeID;

		bool loadAssimpModel(const std::filesystem::path& path);

	public:
		using IAsset::IAsset;
		static AssetTypeID type() { return s_typeID; }

		//Data
		ModelData data;

		/*
		virtual bool create(const std::string& name) override {
			//AssetManager::newModel();
			return true;
		}
		*/
		virtual bool importFromFile(const std::filesystem::path& path) override;

		
		virtual bool load() override;
		virtual bool write() override;


	};

}