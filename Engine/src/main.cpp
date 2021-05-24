#include <RenderWindow.hpp>
//#include "ECS/ECSCoordinator.h"
#include "ECS\Entity.h"
#include "Tools\StopWatch.h"
#include "Tools\ScopeTimer.h"

#include <string>

struct B {
	char name[64];

	uint32_t nameSize;
};

struct A {
	int value;
	size_t size;
	B b;
	unsigned long long id;
};

class MySystem : public System {
	virtual void update(float dt) override {

	}
};

class MySystem1 : public System {
	virtual void update(float dt) override {

	}
};

class MySystem2 : public System {
	virtual void update(float dt) override {

	}
};

int main() {

	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	RenderWindow::init();
	RenderWindow window(1000, 600, "Hello world");
	ECSCoordinator c;


	c.registerComponent<A>();
	c.registerComponent<B>();

	ComponentType aType= c.getComponentType<A>();
	ComponentType bType = c.getComponentType<B>();
	ComponentMask signature;
	signature.set(aType);
	signature.set(bType);
	c.registerSystem<MySystem>(signature);

	signature.reset();
	signature.set(aType);
	c.registerSystem<MySystem1>(signature);
	
	signature.reset();
	signature.set(bType);
	c.registerSystem<MySystem2>(signature);
	
	size_t add = 256 << 7;
	
	std::vector<EntityID> m_entities(add);
	{
		ScopeTimer s("created " + std::to_string(add));
		for (int i = 0; i < add; i++) {
			m_entities[i] = c.createEntity();
		}
	}


	/*
	while (window.isOpen()) {
		window.pollEvents();


	}
	*/
	
	RenderWindow::cleanup();

	return 0;
}