#include "pch.h"
#include "IAsset.h"

namespace sa {
	void IAsset::addDependency(const sa::ProgressView<bool>& progress) {
		m_progress.addDependency(&progress);
	}

	void IAsset::setCompletionCount(unsigned int count) {
		m_progress.setMaxCompletionCount(count);
	}

	void IAsset::incrementProgress() {
		m_progress.increment();
	}

	tf::Future<void> IAsset::runTaskflow(tf::Taskflow& tf) {
		return s_taskExecutor.run(tf);
	}

	IAsset::IAsset(const AssetHeader& header)
		: m_isLoaded(false)
		, m_name("New Asset")
		, m_refCount(0)
		, m_header(header)
	{

	}

	IAsset::~IAsset() {
		m_progress.wait();
	}

	bool IAsset::create(const std::string& name) {
		m_name = name;
		m_isLoaded = true;
		return true;
	}

	bool IAsset::importFromFile(const std::filesystem::path& path) {
		if (!std::filesystem::exists(path)) {
			SA_DEBUG_LOG_ERROR("File path does not exist ", path);
			return false;
		}
		m_isLoaded = onImport(path);
		return m_isLoaded;
	}

	bool IAsset::load(AssetLoadFlags flags) {
		bool force = (flags & AssetLoadFlagBits::FORCE) == AssetLoadFlagBits::FORCE;
		m_refCount++;

		if (m_isLoaded && !force)
			return false;
		if (m_assetPath.empty())
			return false;
		
		auto path = m_assetPath;
		auto future = s_taskExecutor.async([=]() {
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
		});
		m_progress.setFuture(future.share());
		return true;
	}

	bool IAsset::write(AssetWriteFlags flags) {
		if (!m_isLoaded)
			return false;
		if (m_assetPath.empty())
			return false;
		auto path = m_assetPath;
		auto future = s_taskExecutor.async([=]() {
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
		});
		m_progress.setFuture(future.share());
		return true;
	}

	bool IAsset::release() {
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

	bool IAsset::isLoaded() const {
		return m_isLoaded;
	}

	const ProgressView<bool>& IAsset::getProgress() const {
		return m_progress;
	}

	AssetTypeID IAsset::getType() const {
		return m_header.type;
	}

	const std::string& IAsset::getName() const {
		return m_name;
	}

	const std::filesystem::path& IAsset::getAssetPath() const {
		return m_assetPath;
	}

	void IAsset::setAssetPath(const std::filesystem::path& assetPath) {
		m_assetPath = assetPath;
		m_name = m_assetPath.filename().replace_extension().generic_string();
	}

	void IAsset::setHeader(const AssetHeader& header) {
		m_header = header;
	}

	const AssetHeader& IAsset::getHeader() const {
		return m_header;
	}

	const UUID& IAsset::getID() const {
		return m_header.id;
	}

	uint32_t IAsset::getReferenceCount() const {
		return m_refCount;
	}

	AssetHeader IAsset::readHeader(std::ifstream& file) {
		AssetHeader header = {};
		file.read((char*)&header, sizeof(AssetHeader));
		return header;
	}

	void IAsset::writeHeader(const AssetHeader& header, std::ofstream& file) {
		file.write((char*)&header, sizeof(AssetHeader));
	}

	void IAsset::waitAllAssets() {
		s_taskExecutor.wait_for_all();
	}

}