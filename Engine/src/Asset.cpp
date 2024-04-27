#include "pch.h"
#include "Assets/Asset.h"

#include "Lua/LuaAccessable.h"
#include "AssetManager.h"

namespace sa {
	class WorkerInterface : public tf::WorkerInterface
	{
		void scheduler_prologue(tf::Worker& worker) override {};
		void scheduler_epilogue(tf::Worker& worker, std::exception_ptr ptr) override {
			if (ptr) {
				std::rethrow_exception(ptr);
			}
		};
	};

	tf::Executor Asset::s_taskExecutor = tf::Executor(std::thread::hardware_concurrency(), std::make_shared<WorkerInterface>());

	bool Asset::loadCompiledAsset(const std::filesystem::path& path, AssetLoadFlags flags) {
		SA_DEBUG_LOG_INFO("Began loading compiled asset ", m_name, " from ", path);
		std::ifstream file(path, std::ios::binary);
		if (!file.good()) {
			file.close();
			throw std::runtime_error("Failed to open file " + path.generic_string());
		}
		file.seekg(m_header.contentOffset);
		std::vector<byte_t> buffer(m_header.size);
		file.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
		ByteStream byteStream(buffer.data(), buffer.size());
		const bool success = onLoadCompiled(byteStream, flags);
		file.close();

		if (success)
			SA_DEBUG_LOG_INFO("Finished loading ", m_name, " from ", path);
		else
			SA_DEBUG_LOG_WARNING("Could not load ", m_name, " to ", path);

		return success;
	}

	bool Asset::writeCompiledAsset(const std::filesystem::path& path, AssetWriteFlags flags) {
		SA_DEBUG_LOG_INFO("Began compiling ", m_name, " to ", path);
		std::ofstream file(path, std::ios::binary);
		if (!file.good()) {
			file.close();
			throw std::runtime_error("Failed to open file " + path.generic_string());
		}
		const auto headerPos = file.tellp();
		const std::streampos contentPos = sizeof(AssetHeader);

		file.seekp(contentPos);
		ByteStream byteStream(256);
		const bool success = onCompile(byteStream, flags);
		file.write(reinterpret_cast<const char*>(byteStream.data()), byteStream.size());

		// Calculate size and write header
		m_header.size = file.tellp() - contentPos;
		m_header.contentOffset = contentPos;
		file.seekp(headerPos);
		WriteHeader(m_header, file);
		file.close();

		if (success) {
			SA_DEBUG_LOG_INFO("Finished Writing ", m_name, " to ", path);
		}
		else {
			SA_DEBUG_LOG_WARNING("Could not write ", m_name, " to ", path);
		}

		return success;
	}

	bool Asset::loadAsset(const std::filesystem::path& path, AssetLoadFlags flags) {
		SA_DEBUG_LOG_INFO("Began loading asset ", m_name, " from ", path);
		std::filesystem::path metaFileName = getAssetPath();
		metaFileName.replace_extension(SA_META_ASSET_EXTENSION);
		try {
			SA_DEBUG_LOG_INFO("Parsing metafile ", metaFileName, " for asset ", m_name);

			simdjson::padded_string jsonStr = simdjson::padded_string::load(metaFileName.generic_string());
			simdjson::ondemand::parser parser;
			auto doc = parser.iterate(jsonStr);
			simdjson::ondemand::object object = doc.get_object();
			m_header.id = object["id"].get_uint64().take_value();
			m_header.type = object["type"].get_uint64().take_value();
			JsonObject jsonObject;

			const bool success = onLoad(jsonObject, flags);
			if (success) {
				SA_DEBUG_LOG_INFO("Finished loading ", m_name, " from ", path);
			}
			else {
				SA_DEBUG_LOG_WARNING("Could not load ", m_name, " to ", path);
			}
			
			return success;
		}
		catch (const std::exception& e) {
			SA_DEBUG_LOG_ERROR("Failed to parse meta file ", metaFileName.generic_string(), ": ", e.what());
			return false;
		}
	}

	bool Asset::writeAsset(const std::filesystem::path& path, AssetWriteFlags flags) {
		SA_DEBUG_LOG_INFO("Began writing asset ", m_name, " from ", path);
		const bool success = onWrite(flags);
		if (success) {
			std::filesystem::path metaFileName = path;
			metaFileName.replace_extension(SA_META_ASSET_EXTENSION);
			WriteMetaFile(metaFileName, m_header);

			SA_DEBUG_LOG_INFO("Finished writing ", m_name, " from ", path);
		}
		else {
			SA_DEBUG_LOG_WARNING("Could not write ", m_name, " to ", path);
		}
		return success;
	}

	void Asset::addDependency(const sa::ProgressView<bool>& progress) {
		m_progress.addDependency(&progress);
	}

	void Asset::setCompletionCount(unsigned int count) {
		m_progress.setMaxCompletionCount(count);
	}

	void Asset::incrementProgress() {
		m_progress.increment();
	}

	tf::Future<void> Asset::runTaskflow(tf::Taskflow& tf) {
		return s_taskExecutor.run(tf);
	}

	Asset::Asset(const AssetHeader& header, bool isCompiled)
		: m_isLoaded(false)
		, m_name("New Asset")
		, m_refCount(0)
		, m_header(header)
		, m_isCompiled(isCompiled)
	{
		if (m_isCompiled) {
			m_loadFunction = [&](const std::filesystem::path& path, AssetLoadFlags flags) -> bool { return loadCompiledAsset(path, flags); };
			m_writeFunction = [&](const std::filesystem::path& path, AssetWriteFlags flags) -> bool { return writeCompiledAsset(path, flags); };
		}
		else {
			m_loadFunction = [&](const std::filesystem::path& path, AssetLoadFlags flags) -> bool { return loadAsset(path, flags); };
			m_writeFunction = [&](const std::filesystem::path& path, AssetWriteFlags flags) -> bool { return writeAsset(path, flags); };
		}

	}

	Asset::~Asset() {
		m_progress.wait();
	}

	void Asset::initialize(const std::filesystem::path& fileName, const std::filesystem::path& assetDirectory) {
		m_assetPath.clear();
		if (!assetDirectory.empty()){
			m_assetPath = assetDirectory / fileName;
		}
		m_name = fileName.stem().generic_string();
		m_isLoaded = true;
	}

	bool Asset::importFromFile(const std::filesystem::path& path, const std::filesystem::path& assetDirectory) {
		m_name = path.stem().generic_string();

		m_assetPath.clear();
		if (!assetDirectory.empty()) {	
			auto filename = path.filename().replace_extension(".asset");
			m_assetPath = assetDirectory / filename; // The path the asset will write to
		}

		if (!std::filesystem::exists(path)) {
			SA_DEBUG_LOG_ERROR("File path does not exist ", path);
			return false;
		}
		m_isLoaded = onImport(path);
		return m_isLoaded;
	}

	bool Asset::hold() {
		if (++m_refCount == 1) {
			return load();
		}
		return false;
	}

	bool Asset::load(AssetLoadFlags flags) {
		if (m_assetPath.empty())
			return false;
		auto path = m_assetPath;
		auto future = s_taskExecutor.async([=]() {
			try
			{
				std::lock_guard<std::mutex> lock(m_mutex);
				m_progress.reset();
				
				m_isLoaded = m_loadFunction(path, flags);
			}
			catch (std::exception& e)
			{
				SA_DEBUG_LOG_ERROR("[Asset failed load] (", m_name, " <- ", path, ") ", e.what());
				return false;
			}
			return m_isLoaded.load();
		});
		m_progress.setFuture(future.share());
		return true;
	}

	bool Asset::write(AssetWriteFlags flags) {
		std::lock_guard<std::mutex> lock(m_mutex);
		if (!m_isLoaded)
			return false;
		if (m_assetPath.empty())
			return false;
		if (isFromPackage()) {
			SA_DEBUG_LOG_ERROR("Can not write asset to asset package! Recreate the asset package instead or set new asset path");
			return false;
		}

		auto path = m_assetPath;
		auto future = s_taskExecutor.async([=]() {
			try
			{
				std::lock_guard<std::mutex> lock(m_mutex);
				m_progress.reset();
				if (!m_isLoaded)
					return false;

				const bool success = m_writeFunction(path, flags);

				return success;
			}
			catch (std::exception& e)
			{
				SA_DEBUG_LOG_ERROR("[Asset failed write] (", m_name, " -> ", path, ") ", e.what());
			}
			return false;
		});
		m_progress.setFuture(future.share());
		return true;
	}

	bool Asset::release() {
		auto value = m_refCount.load();
		while (value > 0) {
			if (m_refCount.compare_exchange_weak(value, value - 1, std::memory_order_relaxed))
				break;
		}
		if(m_refCount == 0 && m_isLoaded) {
			m_progress.wait();
			SA_DEBUG_LOG_INFO("Unloading ", m_name);
			m_isLoaded = !onUnload();
			return !m_isLoaded;
		}
		return !m_isLoaded;
	}

	bool Asset::compile(const std::filesystem::path& outputPath) {
		std::lock_guard<std::mutex> lock(m_mutex);
		if (!m_isLoaded)
			return false; 
		auto path = outputPath;
		if (path.empty()) {
			path = getAssetPath();
			path.replace_extension(SA_ASSET_EXTENSION);
		}
		auto future = s_taskExecutor.async([=]() {
			try
			{
				std::lock_guard<std::mutex> lock(m_mutex);
				m_progress.reset();
				if (!m_isLoaded)
					return false;

				const bool success = writeCompiledAsset(path, 0);
				
				return success;
			}
			catch (std::exception& e)
			{
				SA_DEBUG_LOG_ERROR("[Asset failed write] (", m_name, " -> ", path, ") ", e.what());
			}
			return false;
		});
		m_progress.setFuture(future.share());
		return true;
	}

	bool Asset::loadCompiled(const std::filesystem::path& path) {
		if (path.empty())
			return false;
		auto future = s_taskExecutor.async([=]() {
			try
			{
				std::lock_guard<std::mutex> lock(m_mutex);
				m_progress.reset();

				m_isLoaded = loadCompiledAsset(path, 0);
			}
			catch (std::exception& e)
			{
				SA_DEBUG_LOG_ERROR("[Asset failed load] (", m_name, " <- ", path, ") ", e.what());
				return false;
			}
			return m_isLoaded.load();
			});
		m_progress.setFuture(future.share());
		return false;
	}

	bool Asset::isLoaded() const {
		return m_isLoaded;
	}

	const ProgressView<bool>& Asset::getProgress() const {
		return m_progress;
	}

	AssetTypeID Asset::getType() const {
		return m_header.type;
	}

	const std::string& Asset::getName() const {
		return m_name;
	}

	void Asset::setName(const std::string& name) {
		m_name = name;
	}

	const std::filesystem::path& Asset::getAssetPath() const {
		return m_assetPath;
	}

	void Asset::setAssetPath(const std::filesystem::path& assetPath) {
		m_assetPath = assetPath;
		if(!isFromPackage())
			m_name = m_assetPath.filename().replace_extension().generic_string();
	}

	void Asset::setHeader(const AssetHeader& header) {
		m_header = header;
	}

	const AssetHeader& Asset::getHeader() const {
		return m_header;
	}

	const UUID& Asset::getID() const {
		return m_header.id;
	}

	uint32_t Asset::getReferenceCount() const {
		return m_refCount;
	}

	bool Asset::isFromPackage() const {
		return m_assetPath.extension() == SA_ASSET_PACKAGE_EXTENSION;
	}

	bool Asset::isFromPackage(const std::filesystem::path& packagePath) const {
		return isFromPackage() && m_assetPath == packagePath;
	}

	AssetHeader Asset::ReadHeader(std::ifstream& file) {
		AssetHeader header = {};
		file.read(reinterpret_cast<char*>(&header), sizeof(AssetHeader));
		return header;
	}

	void Asset::WriteHeader(const AssetHeader& header, std::ofstream& file) {
		file.write(reinterpret_cast<const char*>(&header), sizeof(AssetHeader));
	}

	bool Asset::ReadMetaFile(const std::filesystem::path& path, AssetHeader* header) {
		try {
			simdjson::padded_string jsonStr = simdjson::padded_string::load(path.generic_string());
			simdjson::ondemand::parser parser;
			auto doc = parser.iterate(jsonStr);
			auto object = doc.get_object();
			header->id = object["id"].get_uint64().take_value();
			header->type = object["type"].get_uint64().take_value();
		}
		catch (const std::exception& e) {
			SA_DEBUG_LOG_ERROR("Failed to parse meta file ", path.generic_string(), ": ", e.what());
			return false;
		}
		return true;
	}

	void Asset::WriteMetaFile(const std::filesystem::path& path,  const AssetHeader& header) {
		std::ofstream file(path);
		if (!file.good()) {
			return;
		}
		Serializer serializer;
		serializer.beginObject();
		serializer.value("id", header.id);
		serializer.value("type", header.type);
		serializer.endObject();
		file << serializer.dump();
		file.close();
	}

	void Asset::WaitAllAssets() {
		s_taskExecutor.wait_for_all();
	}

}
