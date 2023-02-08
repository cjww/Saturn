#pragma once

#include <Resources\ResourceManager.hpp>
#include <filesystem>

#include "UUID.h"
#include "ProgressView.h"
namespace sa {
	
	enum class AssetType : uint8_t {
		MODEL,
		SCRIPT,
		MATERIAL,
		TEXTURE,
		OTHER
	};

	struct AssetHeader {
		/*
			----Header----
			ID: 1
			Size: 256
			Offset: 64
			Type: "Model"
		*/
		UUID id;
		size_t size = 0;
		std::streampos offset = 0;
		AssetType type;
	};

	class IAsset {
	protected:
		AssetHeader m_header;
		std::atomic_bool m_isLoaded;
		
		// displayed in editor
		std::string m_name; 
		uint32_t m_refCount;

		std::filesystem::path m_assetPath;

		ProgressView<bool> m_progress;
		tf::Executor m_taskExecutor;

	public:
		IAsset();
		IAsset(const AssetHeader& header);
		IAsset(const UUID& id);

		virtual ~IAsset();

		virtual bool create(const std::string& name) { return false; }
		virtual bool importFromFile(const std::filesystem::path& path) { return false; }

		//virtual bool loadFromPackage(std::ifstream& file) = 0;
		//virtual bool loadFromFile(const std::filesystem::path& path) = 0;

		//virtual bool writeToPackage(std::ofstream& file) = 0;
		//virtual bool writeToFile(const std::filesystem::path& path) = 0;

		virtual bool load() = 0;
		virtual bool write() = 0;


		bool isLoaded() const;
		const ProgressView<bool>& getProgress() const;
		AssetType getType() const;

		const std::string& getName() const;
		const std::filesystem::path& getAssetPath() const;
		void setAssetPath(const std::filesystem::path& assetPath);

		void setHeader(const AssetHeader& header);
		const AssetHeader& getHeader() const;
		
		static AssetHeader readHeader(std::ifstream& file);
		static void writeHeader(const AssetHeader& header, std::ofstream& file);


	};
}
