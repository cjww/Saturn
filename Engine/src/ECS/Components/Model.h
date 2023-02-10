#pragma once

#include "AssetManager.h"

#include "ECS/ComponentBase.h"

namespace comp {
	class Model : public sa::LuaAccessable {
	public:
		sa::UUID modelID = UINT64_MAX;

		Model() = default;

		virtual void serialize(sa::Serializer& s) override;
		virtual void deserialize(void* pDoc) override;


		static void reg();

	};
}