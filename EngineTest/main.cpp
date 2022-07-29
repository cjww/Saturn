#include <filesystem>


#include <Engine.h>
#include <Tools\Clock.h>
#include <Tools\ScopeTimer.h>

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
	//entity.addComponent<comp::Script>();
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
	camera.setPosition({ 0, 0, 1 });
	camera.lookAt({ 0, 0, 0 });
	engine.getCurrentScene()->addActiveCamera(&camera);

	for (int i = 0; i < 1000; i++) {
		//createTriangleEntity(engine);
		createQuad(engine);
	}

	engine.createSystemScript("test.lua");
	

	sa::Entity quad = engine.getCurrentScene()->createEntity("Actor");
	quad.addComponent<comp::Transform>();
	quad.addComponent<comp::Script>();
	quad.addComponent<comp::Model>()->modelID = sa::AssetManager::get().loadQuad();

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
		camera.setPosition(camera.getPosition() - camera.getForward() * dt);
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