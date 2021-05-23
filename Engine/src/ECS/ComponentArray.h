#pragma once
//#include <array>
#include <vector>
#include <unordered_map>
#include <iostream>

#include "EntityFactory.h"

class ComponentArrayBase {
public:
	virtual void remove(EntityID entity) = 0;
};

template<typename T>
class ComponentArray : public ComponentArrayBase {
private:
	// Stores one Component per allowed entity
	//std::array<T, MAX_ENTITY_COUNT> m_data;
	std::vector<T> m_data;
	// How many components are actually used
	size_t m_count;

	// map entityID to an index in the m_data
	std::unordered_map<EntityID, size_t> m_entityToIndex;
	// map index to an entityID in the m_data
	std::unordered_map<size_t, EntityID> m_indexToEntity;

public:
	ComponentArray();
	EntityID getEntity(size_t index) const;
	size_t getIndex(EntityID entity) const;

	// Returns Component data of this entity, nullptr if there is none
	T* get(EntityID entity);
	
	template<typename ...Args>
	T& insert(EntityID entity, Args... args);

	void remove(EntityID entity);

};

template<typename T>
inline ComponentArray<T>::ComponentArray() : m_count(0) {
	m_data.resize(256);
}

template<typename T>
inline EntityID ComponentArray<T>::getEntity(size_t index) const {
	return m_indexToEntity.at(index);
}

template<typename T>
inline size_t ComponentArray<T>::getIndex(EntityID entity) const {
	if (m_entityToIndex.find(entity) == m_entityToIndex.end()) {
		return -1;
	}
	return m_entityToIndex.at(entity);
}

template<typename T>
inline T* ComponentArray<T>::get(EntityID entity) {
	size_t index = getIndex(entity);
	if (index == -1) {
		return nullptr;
	}
	return &m_data.at(index);
}

template<typename T>
template<typename ...Args>
inline T& ComponentArray<T>::insert(EntityID entity, Args... args) {
	if (m_entityToIndex.find(entity) != m_entityToIndex.end()) {
		throw std::runtime_error("Component already added to this entity!");
	}
	if (m_count == m_data.size()) {
		m_data.resize(m_data.size() << 1);
	}

	size_t index = m_count++;
	m_entityToIndex[entity] = index;
	m_indexToEntity[index] = entity;
	//m_data[index] = T(args...);
	
	return m_data[index];
}

template<typename T>
inline void ComponentArray<T>::remove(EntityID entity) {
	size_t index = getIndex(entity);
	if (index == -1)
		return;
	//m_data[index] = m_data[--m_count]; // move last element to fill new hole
	memcpy(&m_data[index], &m_data[--m_count], sizeof(T));

	// Update index mapping
	EntityID lastEntity = getEntity(m_count);
	m_entityToIndex[lastEntity] = index;
	m_indexToEntity[index] = lastEntity;
	
	m_entityToIndex.erase(entity);
	m_indexToEntity.erase(m_count);
}
