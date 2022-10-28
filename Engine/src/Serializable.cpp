#include "pch.h"
#include "Serializable.h"

#include "simdjson.h"

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

	glm::vec3 Serializer::DeserializeVec3(void* pObj) {
		simdjson::ondemand::object& obj = *(simdjson::ondemand::object*)pObj;
		return { (float)obj["x"].get_double(), (float)obj["y"].get_double(), (float)obj["z"].get_double() };
	}

	glm::vec4 Serializer::DeserializeVec4(void* pObj) {
		simdjson::ondemand::object& obj = *(simdjson::ondemand::object*)pObj;
		return { (float)obj["x"].get_double(), (float)obj["y"].get_double(), (float)obj["z"].get_double(), (float)obj["w"].get_double() };
	}

	glm::quat Serializer::DeserializeQuat(void* pObj) {
		simdjson::ondemand::object& obj = *(simdjson::ondemand::object*)pObj;
		return { (float)obj["w"].get_double(), (float)obj["x"].get_double(), (float)obj["y"].get_double(), (float)obj["z"].get_double() };
	}

}