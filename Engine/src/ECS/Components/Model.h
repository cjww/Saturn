#pragma once

#include "AssetManager.h"

#include "ECS/ComponentBase.h"
#include "Lua/LuaAccessable.h"

namespace comp {
	class Model : public sa::ComponentBase, sa::LuaAccessable {
	public:
		sa::UUID modelID = UINT64_MAX;

		Model() = default;
		Model(const Model&) = default;
		Model(Model&&) = default;
		Model& operator=(const Model& other);
		Model& operator=(Model&&) noexcept = default;


		virtual void serialize(sa::Serializer& s) override;
		virtual void deserialize(void* pDoc) override;

		virtual void onDestroy(sa::Entity* e) override;


		static void reg();

	};
}
