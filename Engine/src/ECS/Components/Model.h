#pragma once

#include "AssetManager.h"

#include "ECS/ComponentBase.h"

namespace comp {
	struct Model : public sa::LuaAccessable {
		ResourceID modelID = NULL_RESOURCE;

		Model() = default;
		Model(const std::string& name);


		virtual void serialize(sa::Serializer& s) override;
		virtual void deserialize(void* pDoc) override;


		static void reg();

	};
}