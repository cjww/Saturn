#pragma once

#include "ProgressView.h"
#include "AssetManager.h"

namespace sa {

	template<typename T>
	class AssetHolder {
	private:
		UUID m_id;
		T* m_pAsset;

		static T* getDefaultAsset();

	public:
		AssetHolder();
		AssetHolder(const UUID& id);
		AssetHolder(T* pAsset);
		~AssetHolder();
		AssetHolder(const AssetHolder& other);
		AssetHolder(AssetHolder&& other) noexcept;
		AssetHolder& operator=(const AssetHolder& other);
		AssetHolder& operator=(AssetHolder&& other);
		AssetHolder& operator=(const UUID& id);
		AssetHolder& operator=(T* pAsset);


		operator bool();

		T* getAsset() const;

		const ProgressView<bool>* getProgress() const;
		const char* getName() const;

		const UUID& getID() const;
		static AssetTypeID getTypeID();


	};


	template <typename T>
	inline T* AssetHolder<T>::getDefaultAsset() {
		return nullptr;
	}

	template <>
	inline MaterialShader* AssetHolder<MaterialShader>::getDefaultAsset() {
		return AssetManager::get().getDefaultMaterialShader();
	}

	template <>
	inline Material* AssetHolder<Material>::getDefaultAsset() {
		return AssetManager::get().getDefaultMaterial();
	}

	template <typename T>
	AssetHolder<T>::AssetHolder()
		: m_id(0)
		, m_pAsset(nullptr)
	{

	}

	template <typename T>
	AssetHolder<T>::AssetHolder(const UUID& id) {
		m_id = id;
		m_pAsset = AssetManager::get().getAsset<T>(id);
		m_pAsset->hold();
	}

	template <typename T>
	AssetHolder<T>::AssetHolder(T* pAsset) {
		m_pAsset = pAsset;
		m_id = pAsset->getID();
		m_pAsset->hold();

	}

	template <typename T>
	AssetHolder<T>::~AssetHolder() {
		if (m_pAsset)
			m_pAsset->release();
	}

	template <typename T>
	AssetHolder<T>::AssetHolder(const AssetHolder& other) {
		m_id = other.m_id;
		m_pAsset = other.m_pAsset;
		m_pAsset->hold();
	}

	template <typename T>
	AssetHolder<T>::AssetHolder(AssetHolder&& other) noexcept {
		m_id = other.m_id;
		m_pAsset = other.m_pAsset;

		other.m_pAsset = nullptr;
		other.m_id = 0;
	}

	template <typename T>
	AssetHolder<T>& AssetHolder<T>::operator=(const AssetHolder& other) {
		if (&other == this)
			return *this;

		if (m_pAsset)
			m_pAsset->release();

		m_id = other.m_id;
		m_pAsset = other.m_pAsset;

		if (m_pAsset)
			m_pAsset->hold();

		return *this;
	}

	template <typename T>
	AssetHolder<T>& AssetHolder<T>::operator=(AssetHolder&& other) {
		if (&other == this)
			return *this;

		m_id = other.m_id;
		m_pAsset = other.m_pAsset;

		other.m_pAsset = nullptr;
		other.m_id = 0;

		return *this;
	}

	template <typename T>
	AssetHolder<T>& AssetHolder<T>::operator=(const UUID& id) {
		if (m_id == id)
			return *this;
		if (m_pAsset)
			m_pAsset->release();
		m_id = id;
		m_pAsset = AssetManager::get().getAsset<T>(id);
		if (m_pAsset) {
			m_pAsset->hold();
		}
		else {
			SA_DEBUG_LOG_WARNING("Invalid ", AssetManager::get().getAssetTypeName(getTypeID()), " Asset ID ", id, ", assigning default");
			*this = getDefaultAsset();
		}
		return *this;
	}

	template <typename T>
	AssetHolder<T>& AssetHolder<T>::operator=(T* pAsset) {
		if (m_pAsset == pAsset)
			return *this;

		if (m_pAsset)
			m_pAsset->release();

		m_id = 0;
		m_pAsset = pAsset;

		if (m_pAsset) {
			m_id = m_pAsset->getID();
			m_pAsset->hold();
		}
		return *this;
	}

	template <typename T>
	AssetHolder<T>::operator bool() {
		return m_pAsset != nullptr;
	}

	template <typename T>
	T* AssetHolder<T>::getAsset() const {
		if (m_pAsset && m_pAsset->getProgress().isDone() && m_pAsset->isLoaded())
			return m_pAsset;
		return nullptr;
	}

	template <typename T>
	const ProgressView<bool>* AssetHolder<T>::getProgress() const {
		if (m_pAsset)
			return &m_pAsset->getProgress();
		return nullptr;
	}

	template <typename T>
	const char* AssetHolder<T>::getName() const {
		if (m_pAsset)
			return m_pAsset->getName();
		return "None";
	}

	template <typename T>
	const UUID& AssetHolder<T>::getID() const {
		return m_id;
	}

	template <typename T>
	AssetTypeID AssetHolder<T>::getTypeID() {
		return AssetManager::get().getAssetTypeID<T>();
	}

}
