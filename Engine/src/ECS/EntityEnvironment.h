#pragma once

#include <sol\environment.hpp>

namespace sa {
	namespace details {

		class EntityEnvironment {
		private:
			sol::environment m_env;
		public:
			EntityEnvironment(const sol::state& state, sol::reference& fallback);


			operator sol::environment() {
				return m_env;
			}

			template<typename T>
			auto operator[](T&& key) {
				return m_env[key];
			}
		};
	}
}