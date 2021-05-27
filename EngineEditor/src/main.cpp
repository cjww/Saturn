#include <RenderWindow.hpp>
//#include "ECS/ECSCoordinator.h"
#include <ECS\Entity.h>
#include <Tools\StopWatch.h>
#include <Tools\ScopeTimer.h>

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
public:
	virtual void update(float dt) override {
		std::cout << "MySystem has " << m_entities.size() << " entities" << std::endl;
	}
};

class MySystem1 : public System {
public:
	virtual void update(float dt) override {
		std::cout << "MySystem1 has " << m_entities.size() << " entities" << std::endl;
	}
};

int main() {

	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	RenderWindow::init();
	RenderWindow window(1000, 600, "Hello world");
	ECSCoordinator c;


	c.registerComponent<A>();
	c.registerComponent<B>();

	ComponentType aType = c.getComponentType<A>();
	ComponentType bType = c.getComponentType<B>();

	ComponentMask signature;
	signature.set(aType);
	signature.set(bType);
	ComponentQuery query(signature);
	MySystem* mySystem = c.registerSystem<MySystem>(query);

	ComponentQuery query1(signature, QueryOperator::OR);
	MySystem1* mySystem1 = c.registerSystem<MySystem1>(query1);
	
	size_t add = 256 << 7;
	
	std::vector<EntityID> m_entities(add);
	{
		ScopeTimer s("created " + std::to_string(add));
		for (int i = 0; i < add; i++) {
			m_entities[i] = c.createEntity();
		}
	}

	{
		ScopeTimer s("added " + std::to_string(add));
		for (int i = 0; i < add; i++) {
			c.addComponent<A>(m_entities[i]);
		}
	}

	printf("%x\n", c.addComponent<A>(m_entities[0]));
	printf("%x\n", c.getComponent<B>(m_entities[0]));
	printf("%x\n", c.getComponent<A>(m_entities[0]));

	c.removeComponent<B>(m_entities[0]);

	mySystem->update(0.0f);
	mySystem1->update(0.0f);


	{
		ScopeTimer s("added " + std::to_string(add));
		for (int i = 0; i < add; i++) {
			c.addComponent<B>(m_entities[i]);
		}
	}

	mySystem->update(0.0f);
	mySystem1->update(0.0f);

	{
		ScopeTimer s("removed " + std::to_string(add));
		for (int i = 0; i < add; i++) {
			c.removeComponent<A>(m_entities[i]);
		}
	}

	{
		ScopeTimer s("destroyed " + std::to_string(add));
		for (int i = 0; i < add; i++) {
			c.destroyEntity(m_entities[i]);
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