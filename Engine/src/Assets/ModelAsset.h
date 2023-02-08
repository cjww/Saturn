#pragma once

#include "IAsset.h"
#include "AssetManager.h"

namespace sa {

	class ModelAsset : public IAsset {
	private:
		ResourceID m_id;

		bool loadAssimpModel(const std::filesystem::path& path);

	public:
		using IAsset::IAsset;

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

		ResourceID getID();
	};

}