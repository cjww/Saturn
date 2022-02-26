#include <Engine.h>
#include <Tools\Clock.h>
#include <filesystem>
#include <Tools\ScopeTimer.h>


void createTriangleEntity(sa::Engine& engine) {
	sa::Entity entity = engine.getCurrentScene()->createEntity();
	comp::Model* model = entity.addComponent<comp::Model>();
	entity.addComponent<comp::Transform>()->position = { (float)(rand() % 20) - (20/2), (float)(rand() % 10), -10};

	model->modelID = sa::ResourceManager::get().createModel();

	{
		sa::ModelData* modelData = sa::ResourceManager::get().getModel(model->modelID);
		std::vector<sa::VertexUV> vertices = {
			{{0, 0, 0, 1}, {0, 0}},
			{{1, 0, 0, 1}, {1, 0}},
			{{0, -1, 0, 1}, {0, 1}},
		};

		std::vector<uint32_t> indices = {
			0, 1, 2
		};

		sa::Mesh mesh;
		mesh.indexBuffer = sa::Buffer(sa::BufferType::INDEX);
		mesh.indexBuffer.write(indices);
		mesh.vertexBuffer = sa::Buffer(sa::BufferType::VERTEX);
		mesh.vertexBuffer.write(vertices);

		modelData->meshes.push_back(mesh);
	}
}

int main() {
	srand(time(NULL));


	sa::Engine engine;
	sa::RenderWindow window(1000, 600, "test");
	
	engine.setup(&window, "project.xml");

	sa::Camera camera(&window);
	camera.setPosition({ 0, 0, 1 });
	camera.lookAt({ 0, 0, 0 });
	engine.getCurrentScene()->addActiveCamera(&camera);

	
	std::cout << std::boolalpha << sa::utils::details::has_member_function<comp::Transform>::value
		<< std::endl;


	for (int i = 0; i < 10; i++) {
		createTriangleEntity(engine);
	}

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
		
		

		/*
		// Sway tip
		auto content = modelData->meshes[0].vertexBuffer.getContent<sa::VertexUV>();
		content[2].position.x = (sin(timer) + 1) * 0.5f;

		modelData->meshes[0].vertexBuffer.write(content);
		*/

		if (generateTimer.getElapsedTime() > 0.5f) {
			generateTimer.restart();
			
			engine.getCurrentScene()->forEach<comp::Model>([](comp::Model& model) {
				
				sa::ModelData* modelData = sa::ResourceManager::get().getModel(model.modelID);

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



		engine.update(dt);
		engine.draw();
		
	}
	window.close();

	engine.cleanup();

	return 0;
}