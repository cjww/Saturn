#include "pch.h"
#include "IAsset.h"

namespace sa {
	bool IAsset::dispatchLoad(std::function<bool(std::ifstream&)> loadFunction) {
		if (m_isLoaded)
			return false;
		if (m_assetPath.empty())
			return false;
		auto future = m_taskExecutor.async([&, loadFunction]() {
			std::lock_guard<std::mutex> lock(m_mutex);
			if (m_isLoaded)
				return false;
			m_progress.reset();
			std::ifstream file(m_assetPath, std::ios::binary);
			if (!file.good()) {
				file.close();
				return false;
			}
			m_header = readHeader(file);

			m_isLoaded = loadFunction(file);
		
			file.close();
			return m_isLoaded.load();
		});
		m_progress.setFuture(future.share());
		return true;
	}

	bool IAsset::dispatchWrite(std::function<bool(std::ofstream&)> writeFunction) {
		if (!m_isLoaded)
			return false;
		if (m_assetPath.empty())
			return false;
		auto future = m_taskExecutor.async([&, writeFunction]() {
			std::lock_guard<std::mutex> lock(m_mutex);
			if (!m_isLoaded)
				return false;
			m_progress.reset();

			std::ofstream file(m_assetPath, std::ios::binary);
			if (!file.good()) {
				file.close();
				return false;
			}
			writeHeader(m_header, file);

			bool success = writeFunction(file);
			file.close();
			return success;
		});
		m_progress.setFuture(future.share());
		return true;
	}

	IAsset::IAsset(const AssetHeader& header)
		: m_isLoaded(false)
		, m_name("New Asset")
		, m_refCount(0)
		, m_header(header)
	{

	}

	IAsset::~IAsset() {

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

}