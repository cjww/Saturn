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
	entity.addComponent<comp::Transform>()->position = { 0, 0, -1 };
	entity.addComponent<comp::Model>()->modelID = sa::ResourceManager::get().loadQuad();
	entity.addComponent<comp::Script>();


	engine.createSystemScript("test.lua");
	

	sa::Entity entity3 = engine.getCurrentScene()->createEntity();
	comp::Model* model = entity3.addComponent<comp::Model>();
	entity3.addComponent<comp::Transform>()->position = { 0, 0, 0 };

	model->modelID = sa::ResourceManager::get().createModel();
	sa::ModelData* modelData = sa::ResourceManager::get().getModel(model->modelID);

	std::vector<sa::VertexUV> vertices = {
		{{0, 0, 0, 1}, {0, 0}},
		{{1, 0, 0, 1}, {1, 0}},
		{{0, -1, 0, 1}, {0, 1}},
	};

	std::vector<uint32_t> indices = {
		1, 2, 3
	};

	sa::Mesh mesh;
	mesh.indexBuffer = sa::Buffer(sa::BufferType::INDEX);
	mesh.indexBuffer.write(indices);
	mesh.vertexBuffer = sa::Buffer(sa::BufferType::VERTEX);
	mesh.vertexBuffer.write(vertices);

	modelData->meshes.push_back(mesh);


	engine.init();

	sa::Clock clock;
	while (window.isOpen()) {
		window.pollEvents();
		
		float dt = clock.restart<float>();

		//camera2.rotate(dt, { 0, 1, 0 });
		entity3.getComponent<comp::Transform>()->position.z -= dt;
		
		engine.update(dt);
		engine.draw();
		
	}
	window.close();

	engine.cleanup();

	return 0;
}