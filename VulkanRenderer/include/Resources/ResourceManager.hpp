#pragma once

#include <queue>
#include <unordered_map>
#include <functional>

#include <queue>
#include <string>

typedef size_t ResourceType;
typedef uint32_t ResourceID;

#define NULL_RESOURCE UINT32_MAX

namespace sa {

	namespace details {

		class BasicResourceContainer {
		protected:
			ResourceID m_nextID = 0;
			std::queue<ResourceID> m_freeIDs;
		public:
			virtual ~BasicResourceContainer() = default;

			virtual void remove(ResourceID id) = 0;
			virtual void clear() = 0;

			ResourceID getUniqueID();

			virtual std::string idToKey(ResourceID id) const = 0;

		};

		template<typename T>
		class ResourceContainer : public BasicResourceContainer {
		private:
			std::unordered_map<ResourceID, std::unique_ptr<T>> m_resources;
			std::unordered_map<std::string, ResourceID> m_keys;
			std::function<void(T* value)> m_cleanupFunction;
		public:
			virtual ~ResourceContainer();

			ResourceID insert(const T& value);

			template<typename ...Args>
			ResourceID insert(const Args& ...args);

			ResourceID insert(const std::string& key, const T& value);

			T* get(ResourceID id) const;
			T* get(const std::string& key) const;

			void remove(ResourceID id) override;
			void clear() override;
			std::string idToKey(ResourceID id) const override;

			void setCleanupFunction(std::function<void(T*)> function);

			ResourceID keyToID(const std::string& key) const;


		};

		template<typename T>
		template<typename ...Args>
		inline ResourceID ResourceContainer<T>::insert(const Args& ...args) {
			ResourceID id = getUniqueID();
			m_resources[id] = std::make_unique<T>(args...);
			return id;
		}

	}

	class ResourceManager {
	private:

		std::unordered_map<ResourceType, std::unique_ptr<details::BasicResourceContainer>> m_containers;


		template<typename T>
		details::ResourceContainer<T>* getContainer();

		template<typename T>
		details::ResourceContainer<T>* tryGetContainer() const;

		ResourceManager() = default;
	public:

		static ResourceManager& get();
		virtual ~ResourceManager();

		template<typename T>
		ResourceID insert(const T& value);

		template<typename T, typename ... Args>
		ResourceID insert(const Args&... args);

		template<typename T>
		ResourceID insert(const std::string& key, const T& value);

		template<typename T>
		void setCleanupFunction(std::function<void(T*)> cleanupFunction);

		template<typename T>
		T* get(ResourceID id) const;

		template<typename T>
		T* get(const std::string& key) const;

		template<typename T>
		void remove(ResourceID id);

		template<typename T>
		void clearContainer();

		void clearAll();

		template<typename T>
		ResourceID keyToID(const std::string& key) const;
	
		std::string idToKey(ResourceID id) const;
	
	};

	template<typename T>
	inline details::ResourceContainer<T>* ResourceManager::getContainer() {
		ResourceType type = std::hash<std::string>()(typeid(T).name());
		if (!m_containers.count(type)) {
			m_containers[type] = std::make_unique<details::ResourceContainer<T>>();
		}
		return static_cast<details::ResourceContainer<T>*>(m_containers.at(type).get());
	}

	template<typename T>
	inline details::ResourceContainer<T>* ResourceManager::tryGetContainer() const {
		ResourceType type = std::hash<std::string>()(typeid(T).name());
		if (!m_containers.count(type)) {
			return nullptr;
		}
		return static_cast<details::ResourceContainer<T>*>(m_containers.at(type).get());
	}

	template<typename T>
	inline ResourceID ResourceManager::insert(const T& value) {
		return getContainer<T>()->insert(value);
	}

	template<typename T, typename ...Args>
	inline ResourceID ResourceManager::insert(const Args& ...args) {
		return getContainer<T>()->insert(args...);
	}

	template<typename T>
	inline ResourceID ResourceManager::insert(const std::string& key, const T& value) {
		return getContainer<T>()->insert(key, value);
	}

	template<typename T>
	inline void ResourceManager::setCleanupFunction(std::function<void(T*)> cleanupFunction) {
		getContainer<T>()->setCleanupFunction(cleanupFunction);
	}

	template<typename T>
	inline T* ResourceManager::get(ResourceID id) const {
		details::ResourceContainer<T>* container = tryGetContainer<T>();
		if (!container)
			return nullptr;
		return container->get(id);
	}

	template<typename T>
	inline T* ResourceManager::get(const std::string& key) const {
		details::ResourceContainer<T>* container = tryGetContainer<T>();
		if (!container)
			return nullptr;
		return container->get(key);
	}

	template<typename T>
	inline void ResourceManager::remove(ResourceID id) {
		details::ResourceContainer<T>* container = tryGetContainer<T>();
		if (!container)
			return;
		container->remove(id);
	}

	template<typename T>
	inline void ResourceManager::clearContainer() {
		ResourceType type = std::hash<std::string>()(typeid(T).name());
		m_containers.erase(type);
	}

	template<typename T>
	inline ResourceID ResourceManager::keyToID(const std::string& key) const {
		details::ResourceContainer<T>* container = tryGetContainer<T>();
		if (!container)
			return NULL_RESOURCE;
		return container->keyToID(key);
	}

	namespace details {
		template<typename T>
		inline ResourceContainer<T>::~ResourceContainer() {
			clear();
		}

		template<typename T>
		inline ResourceID ResourceContainer<T>::insert(const T& value) {
			ResourceID id = getUniqueID();
			m_resources[id] = std::make_unique<T>(value);
			return id;
		}

		template<typename T>
		inline ResourceID ResourceContainer<T>::insert(const std::string& key, const T& value) {
			ResourceID id = insert(value);
			m_keys[key] = id;
			return id;
		}

		template<typename T>
		inline T* ResourceContainer<T>::get(ResourceID id) const {
			if (id == NULL_RESOURCE)
				return nullptr;

			return m_resources.at(id).get();
		}

		template<typename T>
		inline T* ResourceContainer<T>::get(const std::string& key) const {
			if (!m_keys.count(key)) {
				return nullptr;
			}
			return get(m_keys.at(key));
		}

		template<typename T>
		inline void ResourceContainer<T>::remove(ResourceID id) {
			if (id == NULL_RESOURCE)
				return;
			if (m_cleanupFunction) {
				m_cleanupFunction(m_resources.at(id).get());
			}
			m_resources.erase(id);
			m_freeIDs.push(id);
		}

		template<typename T>
		inline void ResourceContainer<T>::clear() {
			if (m_cleanupFunction) {
				for (auto& resPair : m_resources) {
					m_cleanupFunction(resPair.second.get());
				}
			}
			m_resources.clear();
			m_keys.clear();
		}

		template<typename T>
		std::string ResourceContainer<T>::idToKey(ResourceID id) const {
			for (auto& [key, resourceID] : m_keys) {
				if (resourceID == id) {
					return key;
				}
			}
			return "";
		}


		template<typename T>
		inline void ResourceContainer<T>::setCleanupFunction(std::function<void(T*)> function) {
			m_cleanupFunction = function;
		}


		template<typename T>
		inline ResourceID ResourceContainer<T>::keyToID(const std::string& key) const {
			if (!m_keys.count(key)) {
				return NULL_RESOURCE;
			}
			return m_keys.at(key);
		}

	}
}