#include <filesystem>


#include <Engine.h>
#include <Tools\Clock.h>
#include <Tools\ScopeTimer.h>

#include <CameraController.h>

int randomRange(int min, int max) {
	return (rand() % (max - min)) - (max - min)/2;
}

void createTriangleEntity(sa::Engine& engine) {
	sa::Renderer& renderer = sa::Renderer::get();


	sa::Entity entity = engine.getCurrentScene()->createEntity();
	comp::Model* model = entity.addComponent<comp::Model>();
	entity.addComponent<comp::Transform>()->position = { (float)randomRange(-20, 20), (float)randomRange(-20, 20), randomRange(-10, 40)};
	entity.addComponent<comp::Script>();

	ResourceID modelID = sa::ResourceManager::get().insert<sa::ModelData>();
	sa::ModelData* modelData = sa::ResourceManager::get().get<sa::ModelData>(modelID);
	model->modelID = modelID;

	{
		std::vector<sa::VertexUV> vertices = {
			{{0, 0, 0, 1}, {0, 0}},
			{{1, 0, 0, 1}, {1, 0}},
			{{0, -1, 0, 1}, {0, 1}},
		};

		std::vector<uint32_t> indices = {
			0, 1, 2
		};

		sa::Mesh mesh;
		mesh.indexBuffer = renderer.createBuffer(sa::BufferType::INDEX);
		mesh.indexBuffer.write(indices);
		mesh.vertexBuffer = renderer.createBuffer(sa::BufferType::VERTEX);
		mesh.vertexBuffer.write(vertices);

		modelData->meshes.push_back(mesh);
	}
}

void createQuad(sa::Engine& engine) {
	sa::Entity entity = engine.getCurrentScene()->createEntity();
	entity.addComponent<comp::Model>()->modelID = sa::AssetManager::get().loadQuad();
	entity.addComponent<comp::Transform>()->position = { (float)randomRange(-50, 50), (float)randomRange(-50, 50), randomRange(-10, 40) };
	entity.addComponent<comp::Script>();
}

void createBox(sa::Engine& engine) {
	sa::Entity entity = engine.getCurrentScene()->createEntity();
	ResourceID ids[2] = { 
		sa::AssetManager::get().loadModel("Box.dae"),
		sa::AssetManager::get().loadModel("GreenBox.dae")
	};
	int r = rand() % 2;
	entity.addComponent<comp::Model>()->modelID = ids[r];

	entity.addComponent<comp::Script>();
	entity.addComponent<comp::Transform>()->position = { (float)randomRange(-50, 50), (float)randomRange(-50, 50), randomRange(-10, 40) };
}

void createModelEntity(sa::Engine& engine, const char* modelPath, float scale = 1.0f) {
	sa::Entity entity = engine.getCurrentScene()->createEntity();
	entity.addComponent<comp::Model>()->modelID = sa::AssetManager::get().loadModel(modelPath);
	comp::Transform* transform = entity.addComponent<comp::Transform>();
	transform->position = { (float)randomRange(-30, 30), (float)randomRange(-30, 30), randomRange(-10, 40) };
	transform->scale *= scale;
}


void regenerate(sa::Scene* scene) {
	scene->forEach<comp::Model>([](comp::Model& model) {

		sa::ModelData* modelData = sa::AssetManager::get().getModel(model.modelID);

		auto vertices = modelData->meshes[0].vertexBuffer.getContent<sa::VertexUV>();
		auto indices = modelData->meshes[0].indexBuffer.getContent<uint32_t>();

		uint32_t thisIndex = vertices.size();
		sa::VertexUV prevVert = vertices.back();

		sa::VertexUV newVert = prevVert;
		if (newVert.position.x == 0) {
			newVert.position.x += 1;
			indices.push_back(thisIndex - 2);
			indices.push_back(thisIndex);
			indices.push_back(thisIndex - 1);
		}
		else {
			newVert.position.y -= 1;
			newVert.position.x = 0;

			indices.push_back(thisIndex - 2);
			indices.push_back(thisIndex - 1);
			indices.push_back(thisIndex);
		}
		vertices.push_back(newVert);
		modelData->meshes[0].vertexBuffer.write(vertices);
		modelData->meshes[0].indexBuffer.write(indices);


	});
}

int main() {
	srand(time(NULL));


	sa::Engine engine;
	sa::RenderWindow window(1000, 600, "test");
	
	engine.setup(&window);

	sa::Camera camera(&window);
	camera.setPosition({ 0, 0, -1 });
	camera.lookAt({ 0, 0, 0 });
	engine.getCurrentScene()->addActiveCamera(&camera);

	sa::CameraController controller(window, camera);
	
	engine.getCurrentScene()->on<sa::event::ComponentCreated<comp::Light>>([](auto& event, sa::Scene& scene) {
		
	});
	
	sa::Entity light = engine.getCurrentScene()->createEntity("Light");
	comp::Light* lightComp = light.addComponent<comp::Light>();
	lightComp->values.color = SA_COLOR_WHITE;
	lightComp->values.position = { 3, -2, 3};
	lightComp->values.strength = 5.0f;


	sa::Entity light2 = engine.getCurrentScene()->createEntity("Light2");
	lightComp = light2.addComponent<comp::Light>();
	lightComp->values.color = {1.0f, 0.3f, 0.3f, 1.0f};
	lightComp->values.position = { -5, -2.5, 5};
	lightComp->values.strength = 7.0f;


	engine.setScene(*engine.getCurrentScene());
	/*
	sa::Entity entity = engine.getCurrentScene()->createEntity();
	entity.addComponent<comp::Model>()->modelID = sa::AssetManager::get().loadModel("Suzanne.dae");
	entity.addComponent<comp::Transform>();
	*/

	sa::Entity entity1 = engine.getCurrentScene()->createEntity();
	comp::Transform* transform = entity1.addComponent<comp::Transform>();
	transform->position = sa::Vector3(2, 0, 0);
	//transform->scale = { .01f, .01f, .01f };
	transform->scale = { 100.f, 100.f, 100.f };
	//entity1.addComponent<comp::Model>()->modelID = sa::AssetManager::get().loadModel("Survival_BackPack_2/Survival_BackPack.fbx");
	//entity1.addComponent<comp::Model>()->modelID = sa::AssetManager::get().loadModel("survival_guitar_backpack/scene.gltf");
	entity1.addComponent<comp::Model>()->modelID = sa::AssetManager::get().loadModel("adamHead/adamHead.gltf");
	entity1.addComponent<comp::Script>();
	//createModelEntity(engine, "stone_ore_1/stone_ore.fbx");
	//createModelEntity(engine, "May holiday/Model/May holiday.fbx", 0.01f);



	for (int i = 0; i < 1000; i++) {
		//createTriangleEntity(engine);
		//createQuad(engine);
		//createBox(engine);
	}

	engine.createSystemScript("test.lua");
	


	sa::Image image("Box.png");
	sa::Texture2D boxTexture = sa::Renderer::get().createTexture2D(image, false);


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
		
		//entity.getComponent<comp::Transform>()->rotation.y += dt;
		/*
		if (generateTimer.getElapsedTime() > 0.5f) {
			generateTimer.restart();
			regenerate(engine.getCurrentScene());
		}
		*/

		engine.update(dt);


		engine.draw();
		
	}
	window.close();

	engine.cleanup();

	return 0;
}