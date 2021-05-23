#pragma once
#include "ComponentFactory.h"

typedef uint8_t TagMask;

class EntityWrapper {
private:
	// Signature that describes which components are attached to this entity
	ComponentMask m_componentMask;
public:
	EntityWrapper(EntityID id);

	// unique ID
	EntityID id;
	
	TagMask tags; // max 8 tags (i.e 8 bits)

	ComponentMask& getSignature();
	
};

// Wraps this to an easier name
typedef std::shared_ptr<EntityWrapper> Entity;
