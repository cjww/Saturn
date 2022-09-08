
#include "TestLayer.h"
#define SA_PROFILER_ENABLE
#include "Tools\Profiler.h"

namespace sa {

	int TestLayer::randomRange(int min, int max) {
		return (rand() % (max - min)) - (max - min) / 2;
	}

	float TestLayer::randomRange(float min, float max) {
		return randomRange((int)min * 1000, (int)max * 1000) / 1000.f;
	}


	Entity createLight(Engine& engine, Vector3 position, Color color = SA_COLOR_WHITE) {
		sa::Entity light = engine.getCurrentScene()->createEntity("Point Light");
		light.addComponent<comp::Transform>()->position = position + Vector3(0, 0, -0.1f);
		light.addComponent<comp::Model>()->modelID = AssetManager::get().loadQuad();
		
		comp::Light* lightComp = light.addComponent<comp::Light>();
		lightComp->values.color = color;
		lightComp->values.position = position;
		return light;
	}

	Entity TestLayer::createModelEntity(Engine& engine, const std::filesystem::path& modelPath, float scale) {
		Entity entity = engine.getCurrentScene()->createEntity(modelPath.filename().string());

		comp::Model* model = entity.addComponent<comp::Model>();

		m_completions.insert({ entity, sa::AssetManager::get().loadModel(modelPath) });
		
		//m_completions.at(entity).wait();

		if (m_completions.at(entity).isDone()) {
			entity.getComponent<comp::Model>()->modelID = m_completions.at(entity);
			m_completions.erase(entity);
		}

		comp::Transform* transform = entity.addComponent<comp::Transform>();
		transform->position = sa::Vector3(m_row, 0, m_column) * 10.f;
		m_row++;
		if (m_row > 10) {
			m_row = 0;
			m_column++;
		}
		transform->scale *= scale;
		entity.addComponent<comp::Script>();
		/*
		comp::Light* lightComp = entity.addComponent<comp::Light>();
		lightComp->values.color = SA_COLOR_WHITE;
		lightComp->values.position = transform->position + sa::Vector3(3, 2, -3);
		lightComp->values.strength = 10.0f;

		sa::Entity lightBox = engine.getCurrentScene()->createEntity("LightBox");
		lightBox.addComponent<comp::Transform>()->position = lightComp->values.position;
		lightBox.addComponent<comp::Model>()->modelID = AssetManager::get().loadQuad();
		*/

		return entity;
	}

	void TestLayer::onAttach(Engine& engine, RenderWindow& window) {
		m_pEngine = &engine;
		m_pWindow = &window;

		//createModelEntity(engine, "resources/models/Box.gltf");

		
		Entity groundEntity = engine.getCurrentScene()->createEntity("Ground");
		groundEntity.addComponent<comp::Model>()->modelID = AssetManager::get().loadQuad();
		comp::Transform* transform = groundEntity.addComponent<comp::Transform>();
		transform->scale = { 500, 500, 1 };
		transform->rotation = glm::rotate(transform->rotation, glm::radians(-90.f), { 1, 0, 0 });
		transform->position.y -= 1;
		
		/*
		Entity entity = createModelEntity(engine, "resources/models/sponza/scene.gltf");
		entity.removeComponent<comp::Script>();

		createModelEntity(engine, "resources/models/adamHead/adamHead.gltf");
		createLight(engine, Vector3(2.f, 2.f, -3.f));

		*/
		for (int i = 0; i < 4096 * 2; i++) {
			Vector3 pos = Vector3(randomRange(0, 200), 0, randomRange(0, 200));
			Color colors[] = {
				SA_COLOR_WHITE,
				SA_COLOR_BLUE,
				SA_COLOR_RED,
				SA_COLOR_GREEN,
				SA_COLOR_YELLOW,
				SA_COLOR_CYAN,
				SA_COLOR_MAGENTA,
			};

			createLight(engine, pos, colors[rand() % 7]);
		}

		/*
		*/
		for (int i = 0; i < 30; i++) {
			createModelEntity(engine, "resources/models/adamHead/adamHead.gltf");
			createModelEntity(engine, "resources/models/lieutenantHead/lieutenantHead.gltf");
			createModelEntity(engine, "resources/models/Suzanne.dae");
		}

		for (int i = 0; i < 30; i++) {
			createModelEntity(engine, "resources/models/adamHead/adamHead.gltf");
			createModelEntity(engine, "resources/models/lieutenantHead/lieutenantHead.gltf");
		}
		for (int i = 0; i < 30; i++) {
			createModelEntity(engine, "resources/models/adamHead/adamHead.gltf");
		}
		for (int i = 0; i < 30; i++) {
			createModelEntity(engine, "resources/models/steampunk_underwater_explorer/scene.gltf", 0.3f);
		}
		for (int i = 0; i < 30; i++) {
			createModelEntity(engine, "resources/models/viking_room/scene.gltf", 0.2f);
			createModelEntity(engine, "resources/models/steampunk_glasses__goggles/scene.gltf");
		}
		
		//createModelEntity(engine, "models/viking_room/scene.gltf", 0.2f);
		
		engine.createSystemScript("test.lua");
		engine.createSystemScript("test2.lua");




	}

	void TestLayer::onUpdate(float dt) {
		SA_PROFILE_FUNCTION();

		std::queue<Entity> entitiesDone;

		for (const auto& [entity, progress] : m_completions) {
			if (m_infoClock.getElapsedTime() > 0.5f) {
				m_infoClock.restart();
				std::cout << "Entity " << entity.getComponent<comp::Name>()->name << " : " << progress.getProgress() * 100 << "%" << std::endl;
			}
			if (progress.isDone()) {
				entity.getComponent<comp::Model>()->modelID = progress;
				entitiesDone.push(entity);
			}
		}

		while (!entitiesDone.empty()) {
			m_completions.erase(entitiesDone.front());
			entitiesDone.pop();
		}

	}

	void TestLayer::onImGuiRender() {

	}
}