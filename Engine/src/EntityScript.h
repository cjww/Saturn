#pragma once

#include <filesystem>
#include "Serializable.h"

namespace sa {

	struct EntityScript : public Serializable {
		std::string name;
		std::filesystem::path path;
		sol::environment env;
		entt::entity owner;
		std::filesystem::file_time_type lastWriteTime;

		std::unordered_map<std::string, sol::object> serializedData;

		EntityScript(std::string name, std::filesystem::path path, sol::environment env, entt::entity owner, std::filesystem::file_time_type lastWriteTime)
			: name(name)
			, path(path)
			, env(env)
			, owner(owner)
			, lastWriteTime(lastWriteTime)
		{}

		virtual void serialize(Serializer& s) override;
		virtual void deserialize(void* pDoc) override;
	};
}