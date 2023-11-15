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

	void Asset::reg() {
		{
			
			auto type = LuaAccessable::registerType<ProgressView<bool>>("AssetProgressView",
				sol::no_constructor);
			type["getCompletion"] = &ProgressView<bool>::getCompletion;
			type["isDone"] = &ProgressView<bool>::isDone;
			type["isAllDone"] = &ProgressView<bool>::isAllDone;
			type["value"] = sol::property(&ProgressView<bool>::getValue);


		}
		{
			auto type = sa::LuaAccessable::registerType<Asset>("Asset",
				sol::no_constructor);
			type["load"] = &Asset::load;
			type["write"] = &Asset::write;
			type["release"] = &Asset::release;

			type["isLoaded"] = &Asset::isLoaded;

			type["progress"] = sol::property([](const Asset& self) { return &self.getProgress(); });
			type["name"] = sol::property(&Asset::getName);
			type["id"] = sol::property(&Asset::getID);
			type["referenceCount"] = sol::property(&Asset::getReferenceCount);
			type["path"] = sol::property([](const Asset& self) { return self.getAssetPath().generic_string(); });
		}

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

	bool Asset::load(AssetLoadFlags flags) {
		bool shallowForce = (flags & AssetLoadFlagBits::FORCE_SHALLOW) == AssetLoadFlagBits::FORCE_SHALLOW;
		bool force = shallowForce || (flags & AssetLoadFlagBits::FORCE) == AssetLoadFlagBits::FORCE;
		bool noRef = (flags & AssetLoadFlagBits::NO_REF) == AssetLoadFlagBits::NO_REF;
		if (shallowForce)
			flags &= ~(AssetLoadFlagBits::FORCE_SHALLOW | AssetLoadFlagBits::FORCE); // Remove Force flags
		if(!noRef)
			m_refCount++; 

		if (m_isLoaded && !force)
			return false;
		if (m_assetPath.empty())
			return false;
		
		auto path = m_assetPath;
			
		auto future = s_taskExecutor.async([=]() {
			try
			{
				std::lock_guard<std::mutex> lock(m_mutex);
				if (m_isLoaded && !force)
					return false;
				SA_DEBUG_LOG_INFO("Began Loading ", m_name, " from ", path);
				m_progress.reset();
				std::ifstream file(path, std::ios::binary);
				if (!file.good()) {
					file.close();
					return false;
				}
				m_header = readHeader(file);
				if (m_header.version != SA_ASSET_VERSION) {
					SA_DEBUG_LOG_WARNING("Asset versions do not match! ", path, " (", m_header.version, " vs ", SA_ASSET_VERSION, ")");
					m_header.version = SA_ASSET_VERSION;
				}

				m_isLoaded = onLoad(file, flags);

				file.close();
				SA_DEBUG_LOG_INFO("Finished Loading ", m_name, " from ", path);
				return m_isLoaded.load();
			}
			catch(std::exception& e)
			{
				SA_DEBUG_LOG_ERROR("[Asset failed load] (", m_name, " <- ", path, ") ", e.what());
			}
			return false;
		});
		m_progress.setFuture(future.share());
		return true;
	}

	bool Asset::write(AssetWriteFlags flags) {
		if (!m_isLoaded)
			return false;
		if (m_assetPath.empty())
			return false;
		auto path = m_assetPath;
		auto future = s_taskExecutor.async([=]() {
			try
			{
				std::lock_guard<std::mutex> lock(m_mutex);
				if (!m_isLoaded)
					return false;
				SA_DEBUG_LOG_INFO("Began Writing ", m_name, " to ", path);
				m_progress.reset();

				std::ofstream file(path, std::ios::binary);
				if (!file.good()) {
					file.close();
					return false;
				}
				writeHeader(m_header, file);

				bool success = onWrite(file, flags);
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
		if (!m_isLoaded)
			return true;
		if (m_refCount > 0) {
			m_refCount--;
		}
		if(m_refCount == 0) {
			m_progress.wait();
			m_isLoaded = !onUnload();
			return true;
		}
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

	const std::filesystem::path& Asset::getAssetPath() const {
		return m_assetPath;
	}

	void Asset::setAssetPath(const std::filesystem::path& assetPath) {
		m_assetPath = assetPath;
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

	AssetHeader Asset::readHeader(std::ifstream& file) {
		AssetHeader header = {};
		file.read((char*)&header, sizeof(AssetHeader));
		return header;
	}

	void Asset::writeHeader(const AssetHeader& header, std::ofstream& file) {
		file.write((char*)&header, sizeof(AssetHeader));
	}

	void Asset::waitAllAssets() {
		s_taskExecutor.wait_for_all();
	}

}
