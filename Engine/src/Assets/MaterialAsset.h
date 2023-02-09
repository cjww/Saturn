#pragma once

#include "IAsset.h"
#include "Graphics/Material.h"
namespace sa {
	class MaterialAsset : public IAsset {
	private:

	public:
		using IAsset::IAsset;

		Material data;

		virtual bool create(const std::string& name) override;

		virtual bool load() override;
		virtual bool write() override;
	};
}