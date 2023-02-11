#include "pch.h"
#include "Model.h"

namespace comp {

	void Model::serialize(sa::Serializer& s) {
		sa::IAsset* pAsset = sa::AssetManager::get().getAsset(modelID);
		s.value("ID", std::to_string(pAsset->getID()).c_str());
	}

	void Model::deserialize(void* pDoc) {
		simdjson::ondemand::object& obj = *(simdjson::ondemand::object*)pDoc;

		std::string_view strID = obj["ID"].get_string().value();
		char* stopString = NULL;
		modelID = strtoull(strID.data(), &stopString, 10);

		sa::IAsset* pModelAsset = sa::AssetManager::get().getAsset(modelID);
		if (!pModelAsset) {
			SA_DEBUG_LOG_ERROR("Invalid model ID: ", modelID);
			return;
		}
		pModelAsset->load();
	}

	void Model::reg() {
		auto type = registerType<Model>("",
			sol::constructors<Model()>()
			);

		type["id"] = &comp::Model::modelID;
	}
}