#pragma once

#include "IAsset.h"
#include "AssetManager.h"

namespace sa {
	class ModelAsset : public IAsset {
	private:
		ResourceID m_id;
	public:
		using IAsset::IAsset;

		ModelData* data = nullptr;

		

		/*
		virtual bool create(const std::string& name) override {
			//AssetManager::newModel();
			return true;
		}
		*/
		virtual bool importFromFile(const std::filesystem::path& path) override;

		
		virtual bool loadFromPackage(std::ifstream& file) override;
		virtual bool loadFromFile(const std::filesystem::path& path) override;

		virtual bool writeToPackage(std::ofstream& file) override;
		virtual bool writeToFile(const std::filesystem::path& path) override;

		virtual bool load() override;
		virtual bool write() override;

		ResourceID getID();
	};

}