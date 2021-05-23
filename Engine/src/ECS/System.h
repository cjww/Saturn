#pragma once
#include <set>

#include "Entity.h"

// Base class for all Systems
class System {
protected:
	// Stores all entities that curently contain the required Components registered in SystemFactory
	std::vector<std::shared_ptr<EntityWrapper>> m_entities;

	// makes sure these can only be called by SystemFactory
	friend class SystemFactory;
	virtual bool removeEntity(Entity entity);
	virtual bool addEntity(Entity entity);
public:
	System();

	virtual void update(float dt) = 0;



};
