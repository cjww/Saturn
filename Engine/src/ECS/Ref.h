#pragma once

#include "Entity.h"

namespace sa {
	class Ref {
	private:
		std::function<sol::table()> m_retriveFunction;
		std::function<void(const Ref&, Serializer&)> m_serializeFunc;

		bool m_hasReference;
		std::string m_typeStr;
	public:
		Ref(const std::string& type, const Entity& value);
		Ref(const sol::table& type, const Entity& value);
		Ref(const sol::object& type, const sol::nil_t& value);
		
		bool hasReference() const;
		const std::string& getType() const;

		static void reg();

	};

}
