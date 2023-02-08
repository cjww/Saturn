#include "pch.h"
#include "MaterialAsset.h"

namespace sa {
	bool MaterialAsset::create(const std::string& name) {
		m_name = name;

		m_header.type = AssetType::MATERIAL;
		return true;
	}

	bool MaterialAsset::load() {
		return false;
	}

	bool MaterialAsset::write() {
		return false;
	}
}
