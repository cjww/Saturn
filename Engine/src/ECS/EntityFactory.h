#pragma once
#include <queue>
#include <stdexcept>
#include <bitset>
#include <unordered_map>

#define MAX_COMPONENT_COUNT 32

typedef size_t EntityID;
typedef std::bitset<MAX_COMPONENT_COUNT> ComponentMask;

class EntityFactory {
private:
	size_t m_createdEntityCount;
	std::queue<EntityID> m_availableEntities;

	std::unordered_map<EntityID, ComponentMask> m_entitySignatures;

public:
	
	EntityFactory();
	// Creates an entity and gives it a unique ID
	EntityID createEntity();
	// Destroy an entity and returns its ID to the pool of usable IDs
	void destroyEntity(EntityID id);
	
	ComponentMask getEntitySignature(EntityID entity) const;
	void setEntitySignature(EntityID entity, ComponentMask siganture);

	uint32_t getEntityCount() const;
};
