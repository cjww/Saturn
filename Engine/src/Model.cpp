#include "pch.h"
#include "ECS/Components/Model.h"
#include "Assets/ModelAsset.h"

namespace comp {

	Model& Model::operator=(const Model& other) {
		model = other.model;
		return *this;
	}

	void Model::serialize(sa::Serializer& s) {
		s.value("ID", std::to_string(model.getID()).c_str());
	}

	void Model::deserialize(void* pDoc) {
		simdjson::ondemand::object& obj = *(simdjson::ondemand::object*)pDoc;

		std::string_view strID = obj["ID"].get_string().value();
		char* stopString = NULL;

		sa::UUID modelID = strtoull(strID.data(), &stopString, 10);
		model = modelID;

	}

	void Model::onDestroy(sa::Entity* e) {

	}


	void Model::reg() {
		auto type = registerType<Model>("",
			sol::constructors<Model()>()
			);

		//type["id"] = &comp::Model::model.getID();
	}
}
