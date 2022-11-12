#include "pch.h"
#include "Model.h"

namespace comp {

	Model::Model(const std::string& name) {
		sa::AssetManager::get().loadModel(name);
	}

	void Model::serialize(sa::Serializer& s) {
		std::filesystem::path path = sa::ResourceManager::get().idToKey<sa::ModelData>(modelID);
		path = std::filesystem::proximate(path);
		s.value("path", path.generic_string().c_str());
	}

	void Model::deserialize(void* pDoc) {
		simdjson::ondemand::object& obj = *(simdjson::ondemand::object*)pDoc;
		std::string_view pathView = obj["path"].get_string().value();
		if (!pathView.empty()) {
			if (pathView == "Quad") {
				modelID = sa::AssetManager::get().loadQuad();
				return;
			}
			if (pathView == "Box") {
				modelID = sa::AssetManager::get().loadBox();
				return;
			}
			modelID = sa::AssetManager::get().loadModel(pathView);
		}
	}

	void Model::reg() {
		auto type = registerType<Model>("",
			sol::constructors<Model(), Model(const std::string&)>()
			);

		type["id"] = &comp::Model::modelID;
	}
}