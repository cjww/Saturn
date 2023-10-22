#include "pch.h"
#include "Serializable.h"

#include "../../DX11Renderer/include/DX11Renderer.h"
#include "Tools/Logger.hpp"


namespace sa {
	Serializer::Serializer()
		: m_depth(0)
		, m_tabSize(6)
		, m_hasElements(false)
	{
	}

	void Serializer::beginScope(const std::string& key, char start) {
		if (m_hasElements) {
			m_ss << ",";
		}

		if (!key.empty())
			m_ss << "\n" << std::setw(m_depth * m_tabSize + 1) << "\"" << key << "\" : " << start;
		else
			m_ss << "\n" << std::setw(m_depth * m_tabSize + 1) << start;
		m_depth++;
		m_hasElements = false;
	}
	
	void Serializer::endScope(char stop) {
		m_hasElements = true;
		m_depth--;
		m_ss << "\n" << std::setw(m_depth * m_tabSize + 1) << stop;
	}

	void Serializer::value(const std::string& key, sol::userdata userdata) {
		const std::optional<sol::protected_function> serializeFunction = userdata.get<std::optional<sol::protected_function>>("serialize");
		if (!serializeFunction.has_value()) {
			SA_DEBUG_LOG_WARNING("Did not serialize requested userdata: " + key + " not serializable");
			return;
		}
		beginObject(key);
		const auto func = serializeFunction.value();
		const auto result = func(userdata, *this);
		if (!result.valid()) {
			const sol::error err = result;
			SA_DEBUG_LOG_ERROR("Failed to serialize userdata " + key + ": ", err.what());
		}
		endObject();
	}

	void Serializer::beginObject(const std::string& key) {
		beginScope(key, '{');
	}

	void Serializer::endObject() {
		endScope('}');
	}

	void Serializer::beginArray(const std::string& key) {
		beginScope(key, '[');
	}

	void Serializer::endArray() {
		endScope(']');
	}

	std::string Serializer::dump() const {
		return m_ss.str();
	}

	glm::vec2 Serializer::DeserializeVec2(void* pObj) {
		simdjson::ondemand::object& obj = *(simdjson::ondemand::object*)pObj;
		float x = obj.find_field("x").get_double();
		float y = obj.find_field("y").get_double();
		return { x, y };
	}

	glm::vec3 Serializer::DeserializeVec3(void* pObj) {
		simdjson::ondemand::object& obj = *(simdjson::ondemand::object*)pObj;
		float x = obj.find_field("x").get_double();
		float y = obj.find_field("y").get_double();
		float z = obj.find_field("z").get_double();
		return { x, y, z };
	}

	glm::vec4 Serializer::DeserializeVec4(void* pObj) {
		simdjson::ondemand::object& obj = *(simdjson::ondemand::object*)pObj;
		float x = obj.find_field("x").get_double();
		float y = obj.find_field("y").get_double();
		float z = obj.find_field("z").get_double();
		float w = obj.find_field("w").get_double();
		return { x, y, z, w };
	}

	glm::quat Serializer::DeserializeQuat(void* pObj) {
		simdjson::ondemand::object& obj = *(simdjson::ondemand::object*)pObj;
		float w = obj.find_field("w").get_double();
		float x = obj.find_field("x").get_double();
		float y = obj.find_field("y").get_double();
		float z = obj.find_field("z").get_double();
		return { w, x, y, z };
	}

}