#include <Engine.h>
#include <Tools\Clock.h>
#include <filesystem>


int main() {



	sa::Engine engine;
	sa::RenderWindow window(1000, 600, "test");
	
	engine.setup(&window, "project.xml");

	sa::Camera camera(&window);
	camera.setPosition({ 0, 0, 1 });
	camera.lookAt({ 0, 0, 0 });
	engine.getCurrentScene()->addActiveCamera(&camera);

	sa::Entity entity = engine.getCurrentScene()->createEntity();
	entity.addComponent<comp::Transform>()->position = { 0, 0, 0 };
	entity.addComponent<comp::Model>()->modelID = sa::ResourceManager::get().loadQuad();
	entity.addComponent<comp::Script>();


	sa::Entity entity2 = engine.getCurrentScene()->createEntity();
	entity2.addComponent<comp::Transform>()->position = { 1, 0, -1 };
	entity2.addComponent<comp::Model>()->modelID = sa::ResourceManager::get().loadQuad();
	
	entity2.addComponent<comp::Script>();

	engine.createSystemScript("test.lua");
	
	sa::ModelData* data = sa::ResourceManager::get().getModel(entity2.getComponent<comp::Model>()->modelID);
	vr::Texture* tex;


	engine.init();

	sa::Clock clock;
	while (window.isOpen()) {
		window.pollEvents();
		
		float dt = clock.restart<float>();

		//camera2.rotate(dt, { 0, 1, 0 });
		//entity.getComponent<comp::Transform>()->position.z -= dt;
		
		engine.update(dt);
		engine.draw();
		
	}
	window.close();

	engine.cleanup();

	return 0;
}