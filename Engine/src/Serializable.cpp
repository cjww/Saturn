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

	void* Serializable::parse(const std::filesystem::path& path) {
		simdjson::ondemand::parser parser;
		simdjson::padded_string json = simdjson::padded_string::load(path.string());
		simdjson::ondemand::document* doc = new simdjson::ondemand::document;
		*doc = parser.iterate(json);
		return doc;
	}
}