#pragma once

#include "Assets/AssetHolder.h"
#include "Assets/ModelAsset.h"

#include "ECS/ComponentBase.h"
#include "Lua/LuaAccessable.h"

namespace comp {
	class Model : public sa::ComponentBase, sa::LuaAccessable {

	public:
		sa::AssetHolder<sa::ModelAsset> model;

		Model() = default;
		Model(const Model&) = default;
		Model(Model&&) = default;
		Model& operator=(const Model& other);
		Model& operator=(Model&&) noexcept = default;


		virtual void serialize(sa::Serializer& s) override;
		virtual void deserialize(void* pDoc) override;

		
		static void reg();

	};
}
