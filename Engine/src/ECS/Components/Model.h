#pragma once

#include "AssetManager.h"

#include "ECS/ComponentBase.h"

namespace comp {
	class Model : public sa::LuaAccessable {
	public:
		sa::UUID modelID = NULL_RESOURCE;

		Model() = default;
		Model(const std::string& name);


		virtual void serialize(sa::Serializer& s) override;
		virtual void deserialize(void* pDoc) override;


		static void reg();

	};
}