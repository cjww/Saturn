#include <RenderWindow.hpp>
#include "ECS/ECSCoordinator.h"
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

int main() {

	RenderWindow window(1000, 600, "Hello world");

	ECSCoordinator c;

	c.registerComponent<A>();
	c.registerComponent<B>();

	StopWatch watch;
	//sizeof(A);
	for (int i = 0; i < 256 << 4; i++) {

		ScopeTimer t(std::to_string(i));
		watch.start();
		Entity e = c.createEntity();
		c.addComponent<A>(e);
		c.addComponent<B>(e);
	}

	std::cout << watch.getAverageLapDuration().count() << std::endl;

	while (window.isOpen()) {
		window.pollEvents();


	}
	

	return 0;
}