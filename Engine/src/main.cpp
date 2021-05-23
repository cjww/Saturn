#include <RenderWindow.hpp>


int main() {

	RenderWindow window(1000, 600, "Hello world");


	while (window.isOpen()) {
		window.pollEvents();


	}
	

	return 0;
}