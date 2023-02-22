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

	typedef uint32_t AssetLoadFlags;
	enum AssetLoadFlagBits : AssetLoadFlags {
		FORCE = 1, // not going to care if asset is already loaded
	};

	typedef uint32_t AssetWriteFlags;
	enum AssetWriteFlagBits : AssetWriteFlags {

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
	private:
		inline static tf::Executor s_taskExecutor;

		AssetHeader m_header;
		std::atomic_bool m_isLoaded;
		std::atomic_uint32_t m_refCount;
		
		// displayed in editor
		std::string m_name; 

		std::filesystem::path m_assetPath;

		ProgressView<bool> m_progress;
		std::mutex m_mutex;
	protected:
		void addDependency(const sa::ProgressView<bool>& progress);
		
		void setCompletionCount(unsigned int count);
		void incrementProgress();

		tf::Future<void> runTaskflow(tf::Taskflow& tf);

	public:
		IAsset(const AssetHeader& header);

		virtual ~IAsset();

		bool create(const std::string& name);
		bool importFromFile(const std::filesystem::path& path);

		//virtual bool loadFromPackage(std::ifstream& file) = 0;
		//virtual bool loadFromFile(const std::filesystem::path& path) = 0;

		//virtual bool writeToPackage(std::ofstream& file) = 0;
		//virtual bool writeToFile(const std::filesystem::path& path) = 0;
		
		virtual bool onImport(const std::filesystem::path& path) { return false; }

		// [DO NOT USE] Called by load. Do not call directly
		virtual bool onLoad(std::ifstream& file, AssetLoadFlags flags) = 0;
		// [DO NOT USE] Called by load. Do not call directly
		virtual bool onWrite(std::ofstream& file, AssetWriteFlags flags) = 0;
		// [DO NOT USE] Called by load. Do not call directly
		virtual bool onUnload() = 0;


		bool load(AssetLoadFlags flags = 0);
		bool write(AssetWriteFlags flags = 0);
		bool release();

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

		static void waitAllAssets();

	};
}
