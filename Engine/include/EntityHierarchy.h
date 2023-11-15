#pragma once
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include "ECS/Entity.h"

namespace sa {

	class EntityHierarchy {
	private:
		
		std::unordered_map<Entity, std::unordered_set<Entity>> m_children;
		std::unordered_map<Entity, Entity> m_parents;

		bool isParent(const Entity& target, const Entity& parent);

	public:
		EntityHierarchy() = default;

		void setParent(const Entity& target, const Entity& parent);
		void orphan(const Entity& target);
		const Entity& getParent(const Entity& child) const;

		void destroy(const Entity& entity);
		void clear();
		void freeMemory();

		bool hasChildren(const Entity& parent) const;
		bool hasParent(const Entity& child) const;
		
		std::unordered_set<Entity>& getChildren(const Entity& entity);

		void forEachChild(const Entity& rootParent, std::function<void(const Entity&, const Entity&)> func);
		void forEachDirectChild(const Entity& rootParent, std::function<void(const Entity&)> func);

		void forEachParent(std::function<void(const Entity&)> func);

	};

}
