#pragma once
#include <string>
#include <stack>

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

		static void* parse(const std::filesystem::path& path);

		virtual void serialize(Serializer& s) = 0;
		virtual void deserialize(void* pDoc) = 0;
	
	};


}