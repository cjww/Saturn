#include "pch.h"
#include "ECS/Components/Model.h"
#include "Assets/ModelAsset.h"

namespace comp {

	Model& Model::operator=(const Model& other) {
		modelID = other.modelID;
		// make sure this asset is loaded and increase its reference count
		sa::AssetManager::get().getAsset<sa::ModelAsset>(modelID)->load();
		return *this;
	}

	void Model::serialize(sa::Serializer& s) {
		sa::Asset* pAsset = sa::AssetManager::get().getAsset(modelID);
		sa::UUID id = 0;
		if (pAsset) {
			id = pAsset->getID();
		}
		s.value("ID", std::to_string(id).c_str());
	}

	void Model::deserialize(void* pDoc) {
		simdjson::ondemand::object& obj = *(simdjson::ondemand::object*)pDoc;

		std::string_view strID = obj["ID"].get_string().value();
		char* stopString = NULL;

		sa::Asset* pPreviousAsset = sa::AssetManager::get().getAsset(modelID);
		
		modelID = strtoull(strID.data(), &stopString, 10);
		sa::Asset* pModelAsset = sa::AssetManager::get().getAsset(modelID);

		if (pPreviousAsset == pModelAsset) {
			return;
		}
		
		if(pModelAsset)
			pModelAsset->load();

		if (pPreviousAsset)
			pPreviousAsset->release();

	}

	void Model::onDestroy(sa::Entity* e) {
		sa::Asset* pModelAsset = sa::AssetManager::get().getAsset(modelID);
		if (pModelAsset) {
			pModelAsset->release();
		}
	}


	void Model::reg() {
		auto type = registerType<Model>("",
			sol::constructors<Model()>()
			);

		type["id"] = &comp::Model::modelID;
	}
}
