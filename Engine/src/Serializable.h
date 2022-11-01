#pragma once
#include <string>
#include <stack>
#include <iomanip>

#include <glm\vec3.hpp>
#include <glm/gtc/quaternion.hpp>

namespace sa {
	
	class Serializer {
	private:
		bool m_hasElements;
		std::stringstream m_ss;
		
		int m_depth;
		int m_tabSize;

		void beginScope(const std::string& key, char start);
		void endScope(char stop);

	public:
		Serializer();
		~Serializer() = default;

		void beginObject(const std::string& key = "");
		void endObject();

		void beginArray(const std::string& key = "");
		void endArray();

		template<typename T>
		void value(T value);

		template<typename T>
		void value(const std::string& key, T value);

		std::string dump() const;

		static glm::vec3 DeserializeVec3(void* pObj);
		static glm::vec4 DeserializeVec4(void* pObj);
		static glm::quat DeserializeQuat(void* pObj);

	};
	
	template<>
	inline void Serializer::value(const char* value) {
		if (m_hasElements) {
			m_ss << ",";
		}
		m_ss << "\n" << std::setw(m_depth * m_tabSize + 1) << "\"" << value << "\"";
		m_hasElements = true;
	}

	template<>
	inline void Serializer::value(const std::string& key, const char* value) {
		if (m_hasElements) {
			m_ss << ",";
		}
		m_ss << "\n" << std::setw(m_depth * m_tabSize + 1) << "\"" << key << "\" : " << "\"" << value << "\"";
		m_hasElements = true;
	}
	
	template<>
	inline void Serializer::value(bool value) {
		if (m_hasElements) {
			m_ss << ",";
		}
		m_ss << "\n" << std::setw(m_depth * m_tabSize + 5) << (value ? "true" : "false");
		m_hasElements = true;
	}

	template<>
	inline void Serializer::value(const std::string& key, bool value) {
		if (m_hasElements) {
			m_ss << ",";
		}
		m_ss << "\n" << std::setw(m_depth * m_tabSize + 1) << "\"" << key << "\" : " << (value ? "true" : "false");
		m_hasElements = true;
	}

	template<>
	inline void Serializer::value(glm::vec3 v) {
		beginObject();
		value("x", v.x);
		value("y", v.y);
		value("z", v.z);
		endObject();
	}

	template<>
	inline void Serializer::value(const std::string& key, glm::vec3 v) {
		beginObject(key);
		value("x", v.x);
		value("y", v.y);
		value("z", v.z);
		endObject();
	}


	template<>
	inline void Serializer::value(glm::vec4 v) {
		beginObject();
		value("x", v.x);
		value("y", v.y);
		value("z", v.z);
		value("w", v.w);
		endObject();
	}

	template<>
	inline void Serializer::value(const std::string& key, glm::vec4 v) {
		beginObject(key);
		value("x", v.x);
		value("y", v.y);
		value("z", v.z);
		value("w", v.w);
		endObject();
	}

	template<>
	inline void Serializer::value(glm::quat q) {
		beginObject();
		value("w", q.w);
		value("x", q.x);
		value("y", q.y);
		value("z", q.z);
		endObject();
	}

	template<>
	inline void Serializer::value(const std::string& key, glm::quat q) {
		beginObject(key);
		value("w", q.w);
		value("x", q.x);
		value("y", q.y);
		value("z", q.z);
		endObject();
	}


	template<typename T>
	inline void Serializer::value(T value) {
		if (m_hasElements) {
			m_ss << ",";
		}
		std::string str = std::to_string(value);
		m_ss << "\n" << std::setw(m_depth * m_tabSize + str.size()) << str;
		m_hasElements = true;
	}

	template<typename T>
	inline void Serializer::value(const std::string& key, T value) {
		if (m_hasElements) {
			m_ss << ",";
		}
		m_ss << "\n" << std::setw(m_depth * m_tabSize + 1) << "\"" << key << "\" : " << std::to_string(value);
		m_hasElements = true;
	}


	class Serializable {
	public:

		virtual void serialize(Serializer& s) = 0;
		virtual void deserialize(void* pDoc) = 0;
	
	};


}