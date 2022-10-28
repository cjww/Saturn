#pragma once

#include "ECS/ComponentBase.h"

namespace comp {
	struct Name : public sa::ComponentBase {
		std::string name;

		Name() {}
		Name(std::string name) : name(name) {}

		virtual void serialize(sa::Serializer& s) override;
		virtual void deserialize(void* pDoc) override;

	};
}