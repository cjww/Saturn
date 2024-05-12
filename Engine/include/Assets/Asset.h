#pragma once

#include <Resources\ResourceManager.hpp>
#include <filesystem>

#include "UUID.h"
#include "ProgressView.h"

#include "Core.h"
#include "taskflow/taskflow.hpp"

#include "Tools\ByteStream.h"

#include "Serializable.h"

namespace sa {
	
	typedef uint32_t AssetTypeID;
	struct AssetHeader {
		UUID id;
		size_t size = 0;
		std::streampos contentOffset = sizeof(AssetHeader);
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
		
		const bool m_isCompiled;
		std::function<bool(const std::filesystem::path&, AssetLoadFlags)> m_loadFunction;
		std::function<bool(const std::filesystem::path&, AssetWriteFlags)> m_writeFunction;

		bool loadCompiledAsset(const std::filesystem::path& path, AssetLoadFlags flags);
		bool writeCompiledAsset(const std::filesystem::path& path, AssetWriteFlags flags);

		bool loadAsset(const std::filesystem::path& path, AssetLoadFlags flags);
		bool writeAsset(const std::filesystem::path& path, AssetWriteFlags flags);

	protected:
		void addDependency(const sa::ProgressView<bool>& progress);
		
		void setCompletionCount(unsigned int count);
		void incrementProgress();

		tf::Future<void> runTaskflow(tf::Taskflow& tf);

	public:
		Asset(const AssetHeader& header, bool isCompiled);

		virtual ~Asset();

		void initialize(const std::filesystem::path& filename, const std::filesystem::path& assetDirectory);
		
		// [DO NOT USE] Called by load. Do not call directly
		virtual bool onLoad(JsonObject& metaData, AssetLoadFlags flags) = 0;
		virtual bool onLoadCompiled(ByteStream& dataInStream, AssetLoadFlags flags) = 0;

		// [DO NOT USE] Called by write. Do not call directly
		virtual bool onWrite(AssetWriteFlags flags) = 0;
		virtual bool onCompile(ByteStream& dataOutStream, AssetWriteFlags flags) = 0;
		// [DO NOT USE] Called by release. Do not call directly
		virtual bool onUnload() = 0;

		virtual Asset* clone(const std::string& name, const std::filesystem::path& assetDir = "") const = 0;

		//	load()
		//		open file
		//		if compiled
		//			navigate to data chunk
		//			onLoadCompiled(data chunk)
		//		else
		//			parse json object
		//			onLoad(jsonObject)
		//		close file

		//	write()
		//		if compiled
		//			logic error
		//		open file
		//		onWrite(serializer)
		//		file << serializer.data()
		//		close file

		//	AssetManager::compile(assets)
		//		calculate space needed
		//			size += space for all assetHeaders
		//			for each asset
		//				size += asset.getBinaryDataSize()
		//		allocate data
		//		for each asset
		//			asset.compile(data + offset)
		//		write asset count to file
		//		write headers to file
		//		write data blob to file

		//	compile(data)
		//		onCompile(data)

		//	AssetManager::addAsset(path)
		//		if .mtasset
		//			parse json
		//			read type and id
		//			create asset of type
		//		else if .assetpkg
		//			addAssetPackage()
		//		else if extension exists in supportedFiles {
		//			get asset create function
		//			create asset 

		bool hold();
		bool load(AssetLoadFlags flags = 0);
		bool write(AssetWriteFlags flags = 0);
		bool release();

		bool compile(const std::filesystem::path& outputPath = "");
		bool loadCompiled(const std::filesystem::path& path);

		bool isLoaded() const;
		bool isCompiled() const;
		
		const ProgressView<bool>& getProgress() const;
		AssetTypeID getType() const;

		const std::string& getName() const;
		void setName(const std::string& name);
		const std::filesystem::path& getAssetPath() const;
		void setAssetPath(const std::filesystem::path& assetPath);

		std::filesystem::path getMetaFilePath() const;

		void setHeader(const AssetHeader& header);
		const AssetHeader& getHeader() const;
		
		const UUID& getID() const;

		uint32_t getReferenceCount() const;

		bool isFromPackage() const;
		bool isFromPackage(const std::filesystem::path& packagePath) const;

		static AssetHeader ReadHeader(std::ifstream& file);
		static void WriteHeader(const AssetHeader& header, std::ofstream& file);

		static bool ReadMetaFile(const std::filesystem::path& path, AssetHeader* header);
		static void WriteMetaFile(const std::filesystem::path& path, const AssetHeader& header);

		static void WaitAllAssets();

		template<typename T>
		T* cast();

	};


	template<typename T>
	inline T* Asset::cast() {
		return dynamic_cast<T*>(this);
	}
}
