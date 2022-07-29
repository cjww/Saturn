#include "pch.h"
#include "Resources/ResourceManager.hpp"

namespace sa {

	ResourceManager& ResourceManager::get() {
		static ResourceManager instance;
		return instance;
	}

	ResourceManager::~ResourceManager() {
		clearAll();
	}

	void ResourceManager::clearAll() {
		m_containers.clear();
	}

	std::string ResourceManager::idToKey(ResourceID id) const {
		for (auto& container : m_containers) {
			std::string key = container.second->idToKey(id);
			if (!key.empty()) {
				return key;
			}
		}
		return "-";
	}


	namespace details {

		ResourceID BasicResourceContainer::getUniqueID() {
			if (!m_freeIDs.empty()) {
				ResourceID id = m_freeIDs.front();
				m_freeIDs.pop();
				return id;	
			}
			return m_nextID++;
		}

	}

}