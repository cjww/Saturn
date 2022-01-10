#pragma once

namespace sa {
	namespace utils {
		template<typename T>
		const char* getTypeName();

		template<typename T>
		const char* getTypeName() {
			return typeid(T).name() + 7;
		}
	}

}