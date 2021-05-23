#pragma once
#include <queue>
#include <stdexcept>

typedef size_t EntityID;

class EntityFactory {
private:
	size_t m_createdEntityCount;
	std::queue<EntityID> m_availableEntities;
public:
	
	EntityFactory();
	// Creates an entity and gives it a unique ID
	EntityID createEntity();
	// Destroy an entity and returns its ID to the pool of usabe IDs
	void destroyEntity(EntityID id);
	
};
