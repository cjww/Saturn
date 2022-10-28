#include "pch.h"
#include "EntityHierarchy.h"

namespace sa {
	
	bool EntityHierarchy::isParent(const Entity& target, const Entity& parent) {
		if (m_parents.count(target)) {
			if(m_parents.at(target) == parent) {
				return true;
			}
			return isParent(m_parents.at(target), parent);
		}
		return false;
		
	}

	void EntityHierarchy::setParent(const Entity& target, const Entity& parent) {
		// if parent is child to target
		bool isparent = isParent(parent, target);
		if (isparent) {
			parent.orphan();
		}

		m_parents[target] = parent;
		for (auto& [prnt, children] : m_children) {
			size_t e = children.erase(target);
			if (isparent && e) {
				children.emplace(parent);
				m_parents[parent] = prnt;
			}
		}
		
		m_children[parent].emplace(target);
	}

	void EntityHierarchy::orphan(const Entity& target) {
		m_children[m_parents[target]].erase(target);
		m_parents.erase(target);
	}

	const Entity& EntityHierarchy::getParent(const Entity& child) const {
		if (m_parents.count(child))
			return m_parents.at(child);
		return {};
	}

	void EntityHierarchy::destroy(const Entity& entity) {
		auto& childSet = m_children[entity];
		while (!childSet.empty()) {
			auto it = childSet.begin(); 
			Entity e = *it;
			e.destroy();
		}
		orphan(entity);
		m_children.erase(entity);
	}

	void EntityHierarchy::clear() {
		m_children.clear();
		m_parents.clear();
	}

	bool EntityHierarchy::hasChildren(const Entity& parent) const {
		return m_children.count(parent) && !m_children.at(parent).empty();
	}

	bool EntityHierarchy::hasParent(const Entity& child) const {
		return m_parents.count(child) && !m_parents.at(child).isNull();
	}

	std::unordered_set<Entity>& EntityHierarchy::getChildren(const Entity& entity) {
		return m_children[entity];
	}
	
	void EntityHierarchy::forEachChild(const Entity& parent, std::function<void(const Entity&, const Entity&)> func) {
		for (auto& node : m_children[parent]) {
			func(node, parent);
			forEachChild(node, func);
		}
	}

	void EntityHierarchy::forEachDirectChild(const Entity& rootParent, std::function<void(const Entity&)> func) {
		for (auto& node : m_children[rootParent]) {
			func(node);
		}
	}
}