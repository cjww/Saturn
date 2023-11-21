#pragma once

#include <Resources\ResourceManager.hpp>
#include <filesystem>

#include "UUID.h"
#include "ProgressView.h"

#include "Core.h"
#include "taskflow/taskflow.hpp"

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
		FORCE_SHALLOW = 2, // Force only first load in tree
		NO_REF = 4, // ignore this load call
		
	};

	typedef uint32_t AssetWriteFlags;
	enum AssetWriteFlagBits : AssetWriteFlags {

	};

	/*
		Base class for Assets
		All Asset subclasses must fill following conditions:
			* override bool onLoad(std::ifstream& file, AssetLoadFlags flags)
			* override bool onWrite(std::ofstream& file, AssetWriteFlags flags)
			* override bool onUnload()
			* use Asset constructors or using Asset::Asset
	*/
	class Asset {
	private:
		static tf::Executor s_taskExecutor;

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
		Asset(const AssetHeader& header);

		virtual ~Asset();

		static void reg();

		bool create(const std::string& name, const std::filesystem::path& assetDirectory);
		bool importFromFile(const std::filesystem::path& path, const std::filesystem::path& assetDirectory);

		//virtual bool loadFromPackage(std::ifstream& file) = 0;
		//virtual bool loadFromFile(const std::filesystem::path& path) = 0;

		//virtual bool writeToPackage(std::ofstream& file) = 0;
		//virtual bool writeToFile(const std::filesystem::path& path) = 0;
		
		virtual bool onImport(const std::filesystem::path& path) { return false; }

		// [DO NOT USE] Called by load. Do not call directly
		virtual bool onLoad(std::ifstream& file, AssetLoadFlags flags) = 0;
		// [DO NOT USE] Called by write. Do not call directly
		virtual bool onWrite(std::ofstream& file, AssetWriteFlags flags) = 0;
		// [DO NOT USE] Called by release. Do not call directly
		virtual bool onUnload() = 0;


		bool hold();
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

		template<typename T>
		T* cast();

	};


	template<typename T>
	inline T* Asset::cast() {
		return dynamic_cast<T*>(this);
	}
}
