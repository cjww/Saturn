#pragma once

#include "ECS/ComponentBase.h"

namespace comp {
	class Name : public sa::ComponentBase {
	public:
		std::string name;

		Name() {}
		Name(std::string name) : name(name) {}

		virtual void serialize(sa::Serializer& s) override;
		virtual void deserialize(void* pDoc) override;

	};
}