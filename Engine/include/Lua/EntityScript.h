#pragma once

#include <filesystem>
#include "Serializable.h"
#include "Tools/Logger.hpp"

namespace sa {
	
	struct EntityScript : public Serializable {
		std::string name;
		std::filesystem::path path;
		sol::environment env;
		entt::entity owner;
		std::filesystem::file_time_type lastWriteTime;

		std::unordered_map<std::string, sol::object> serializedData;

		std::vector<entt::connection> connections;

		EntityScript() {};

		EntityScript(const std::string& name, const std::filesystem::path& path, const sol::environment& env, entt::entity owner, const std::filesystem::file_time_type& lastWriteTime)
			: name(name)
			, path(path)
			, env(env)
			, owner(owner)
			, lastWriteTime(lastWriteTime)
		{}
		virtual ~EntityScript();

		virtual void serialize(Serializer& s) override;
		virtual void deserialize(void* pDoc) override;

		template<typename ...Args>
		static void Call(const sol::safe_function func, Args&& ...args);

		template<typename Event, auto ...values>
		void listener(const Event& e);

		template<typename Event, auto ...values>
		void bind(entt::dispatcher& dispatcher);

		void disconnect();
	};


	template <typename ... Args>
	void EntityScript::Call(const sol::safe_function func, Args&&... args) {
		auto result = func(args...);
		if (result.status() != sol::call_status::ok) {
			sol::error error = result;
			SA_DEBUG_LOG_ERROR("Error in lua function: ", error.what());
		}
	}

	template <typename Event, auto ...values>
	void EntityScript::listener(const Event& e) {
		sol::safe_function function = env[Event::CallbackName];
		env.set_on(function);
		Call(function, ((&e)->*values) ...);
	}

	template <typename Event, auto... values>
	void EntityScript::bind(entt::dispatcher& dispatcher) {
		const sol::safe_function function = env[Event::CallbackName];
		if (function == sol::nil)
			return;

		connections.emplace_back(dispatcher.sink<Event>()
			.connect<&EntityScript::listener<Event, values...>>(this));
	}

}
