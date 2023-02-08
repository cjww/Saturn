#include "pch.h"
#include "IAsset.h"

namespace sa {
	IAsset::IAsset() 
		: m_isLoaded(false)
		, m_name("New Asset")
		, m_refCount(0)
	{

	}

	IAsset::IAsset(const AssetHeader& header)
		: IAsset()
	{
		m_header = header;
	}


	IAsset::IAsset(const UUID& id) 
		: IAsset() 
	{
		m_header.id = id;
	}

	IAsset::~IAsset() {

	}

	bool IAsset::isLoaded() const {
		return m_isLoaded;
	}

	const ProgressView<bool>& IAsset::getProgress() const {
		return m_progress;
	}

	AssetType IAsset::getType() const {
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

	AssetHeader IAsset::readHeader(std::ifstream& file) {
		AssetHeader header = {};
		file.read((char*)&header, sizeof(AssetHeader));
		return header;
	}

	void IAsset::writeHeader(const AssetHeader& header, std::ofstream& file) {
		file.write((char*)&header, sizeof(AssetHeader));
	}

}