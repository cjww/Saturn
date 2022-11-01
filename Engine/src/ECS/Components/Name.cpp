#include "pch.h"
#include "Name.h"

#include "simdjson.h"

namespace comp {
	void Name::serialize(sa::Serializer& s) {
		
		s.value("name", name.c_str());

	}
	void Name::deserialize(void* pDoc) {
		simdjson::ondemand::object& obj = *(simdjson::ondemand::object*)pDoc;
		name = obj["name"].get_string().value();
	}

}
