#pragma once

#include "Serializable.h"

namespace sa {
	class Entity;

	class ComponentBase : public Serializable {
	public:
	
		virtual void onConstruct(sa::Entity* e) {};
		virtual void onUpdate(sa::Entity* e) {};
		virtual void onDestroy(sa::Entity* e) {};
		virtual void onCopy(sa::Entity* e, sa::Entity* other) {};
	};
}


