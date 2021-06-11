#pragma once
#include <set>
#include "EntityFactory.h"

// Base class for all Systems
class System {
protected:
	// Stores all entities that curently contain the required Components registered in SystemFactory
	std::set<EntityID> m_entitySet;

	// makes sure these can only be called by SystemFactory
	friend class SystemFactory;
	virtual bool removeEntity(EntityID entity);
	virtual bool addEntity(EntityID entity);
public:
	System();
	virtual ~System();
	virtual void update(float dt) = 0;



};
