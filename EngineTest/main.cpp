#include <filesystem>


#include <Engine.h>
#include <Tools\Clock.h>
#include <Tools\ScopeTimer.h>

#include <CameraController.h>

int randomRange(int min, int max) {
	return (rand() % (max - min)) - (max - min)/2;
}

int g_row = 0;
int g_column = 0;

sa::Entity createModelEntity(sa::Engine& engine, const char* modelPath, float scale = 1.0f) {
	sa::Entity entity = engine.getCurrentScene()->createEntity();
	
	auto future = sa::AssetManager::get().loadModelAsync(&entity.addComponent<comp::Model>()->modelID, modelPath);
	
	comp::Transform* transform = entity.addComponent<comp::Transform>();
	transform->position = sa::Vector3(g_row, 0, g_column) * 10.f;
	g_row++;
	if (g_row > 10) {
		g_row = 0;
		g_column++;
	}
	transform->scale *= scale;
	entity.addComponent<comp::Script>();

	comp::Light* lightComp = entity.addComponent<comp::Light>();
	lightComp->values.color = SA_COLOR_WHITE;
	lightComp->values.position = transform->position + sa::Vector3(3, 2, -3);
	lightComp->values.strength = 10.0f;

	return entity;
}


int main() {
	srand(time(NULL));


	sa::Engine engine;
	sa::RenderWindow window(1000, 600, "Saturn");
	
	engine.setup(&window);

	sa::Camera camera(&window);
	camera.setPosition({ 0, 0, -10 });
	camera.lookAt({ 2, 0, 0 });
	engine.getCurrentScene()->addActiveCamera(&camera);

	sa::CameraController controller(window, camera);
	
	createModelEntity(engine, "models/adamHead/adamHead.gltf");
	createModelEntity(engine, "models/lieutenantHead/lieutenantHead.gltf");
	
	createModelEntity(engine, "models/steampunk_underwater_explorer/scene.gltf", 0.5f);
	createModelEntity(engine, "models/steampunk_glasses__goggles/scene.gltf");
	createModelEntity(engine, "models/viking_room/scene.gltf", 0.2f);


	engine.setScene(*engine.getCurrentScene());

	engine.createSystemScript("test.lua");
	

	engine.init();

	sa::Clock clock;
	float timer = 0.0f;
	sa::Clock fpsClock;

	sa::Clock generateTimer;
 	while (window.isOpen()) {
		window.pollEvents();
		float dt = clock.restart();
		timer += dt;

		if (fpsClock.getElapsedTime() > 0.5f) {
			window.setWindowTitle("FPS: " + std::to_string(1 / dt));
			fpsClock.restart();
		}
		
		controller.update(dt);
		

		engine.update(dt);


		engine.draw();
		
	}
	window.close();

	engine.cleanup();

	return 0;
}