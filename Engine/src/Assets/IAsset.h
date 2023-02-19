#pragma once

#include <Resources\ResourceManager.hpp>
#include <filesystem>

#include "UUID.h"
#include "ProgressView.h"

#include "Core.h"
namespace sa {

	typedef uint32_t AssetTypeID;
	struct AssetHeader {
		UUID id;
		size_t size = 0;
		std::streampos offset = 0;
		AssetTypeID type;
		uint16_t version = SA_ASSET_VERSION;
	};

	/*
		Base class interface for Assets
		All IAsset subclasses must fill following conditions:
			* override bool load()
			* override bool write()
			* override private bool unload()
			* friend class AssetManager
			* have a private static AssetTypeID s_typeID
			* have a public static AssetTypeID type() { return s_typeID; }
			* use IAsset constructors or using IAsset::IAsset
	*/
	class IAsset {
	protected:
		inline static tf::Executor s_taskExecutor;

		AssetHeader m_header;
		std::atomic_bool m_isLoaded;
		std::atomic_uint32_t m_refCount;
		
		// displayed in editor
		std::string m_name; 

		std::filesystem::path m_assetPath;

		ProgressView<bool> m_progress;
		std::mutex m_mutex;

		bool dispatchLoad(std::function<bool(std::ifstream&)> loadFunction);
		bool dispatchWrite(std::function<bool(std::ofstream&)> writeFunction);
		virtual bool unload() = 0;
	public:
		IAsset(const AssetHeader& header);

		virtual ~IAsset();

		virtual bool create(const std::string& name) { return false; }
		virtual bool importFromFile(const std::filesystem::path& path) { return false; }

		//virtual bool loadFromPackage(std::ifstream& file) = 0;
		//virtual bool loadFromFile(const std::filesystem::path& path) = 0;

		//virtual bool writeToPackage(std::ofstream& file) = 0;
		//virtual bool writeToFile(const std::filesystem::path& path) = 0;

		virtual bool load() = 0;
		virtual bool write() = 0;
		

		void release();

		bool isLoaded() const;
		const ProgressView<bool>& getProgress() const;
		AssetTypeID getType() const;

		const std::string& getName() const;
		const std::filesystem::path& getAssetPath() const;
		void setAssetPath(const std::filesystem::path& assetPath);

		void setHeader(const AssetHeader& header);
		const AssetHeader& getHeader() const;
		
		const UUID& getID() const;

		uint32_t getReferenceCount() const;

		static AssetHeader readHeader(std::ifstream& file);
		static void writeHeader(const AssetHeader& header, std::ofstream& file);


	};
}
