#include "pch.h"
#include "Assets/Asset.h"

#include "Lua/LuaAccessable.h"

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

	bool Asset::readCompiledAsset(std::ifstream& file, AssetLoadFlags flags) {
		file.seekg(m_header.contentOffset);
		return onLoad(file, flags);
	}

	bool Asset::writeCompiledAsset(std::ofstream& file, AssetWriteFlags flags) {
		const auto headerPos = file.tellp();
		const std::streampos contentPos = sizeof(AssetHeader);

		file.seekp(contentPos);
		const bool success = onWrite(file, flags);

		// Calculate size and write header
		m_header.size = file.tellp() - contentPos;
		m_header.contentOffset = contentPos;
		file.seekp(headerPos);
		WriteHeader(m_header, file);
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

	Asset::Asset(const AssetHeader& header)
		: m_isLoaded(false)
		, m_name("New Asset")
		, m_refCount(0)
		, m_header(header)
	{
		
	}

	Asset::~Asset() {
		m_progress.wait();
	}

	bool Asset::create(const std::string& name, const std::filesystem::path& assetDirectory) {
		m_name = name;
		m_assetPath.clear();
		if (!assetDirectory.empty()) {
			m_assetPath = assetDirectory / (name + ".asset"); // The path the asset will write to
		}
		m_isLoaded = true;
		return true;
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
				SA_DEBUG_LOG_INFO("Began Loading ", m_name, " from ", path);
				
				std::ifstream file(path, std::ios::binary);
				if (!file.good()) {
					file.close();
					throw std::runtime_error("Failed to open file " + path.generic_string());
				}

				m_isLoaded = readCompiledAsset(file, flags);

				file.close();
				SA_DEBUG_LOG_INFO("Finished Loading ", m_name, " from ", path);
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
			throw std::runtime_error("Can not write asset to asset package! Recreate the asset package instead or set new asset path");
		}

		auto path = m_assetPath;
		auto future = s_taskExecutor.async([=]() {
			try
			{
				std::lock_guard<std::mutex> lock(m_mutex);
				m_progress.reset();
				if (!m_isLoaded)
					return false;
				SA_DEBUG_LOG_INFO("Began Writing ", m_name, " to ", path);

				std::ofstream file(path, std::ios::binary);
				if (!file.good()) {
					file.close();
					return false;
				}

				const bool success = writeCompiledAsset(file, flags);


				file.close();
				SA_DEBUG_LOG_INFO("Finished Writing ", m_name, " to ", path);
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

	void Asset::WaitAllAssets() {
		s_taskExecutor.wait_for_all();
	}

}
