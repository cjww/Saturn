
#include "TestLayer.h"
#define SA_PROFILER_ENABLE
#include "Tools\Profiler.h"

namespace sa {

	int TestLayer::randomRange(int min, int max) {
		return (rand() % (max - min)) - (max - min) / 2;
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

		comp::Light* lightComp = entity.addComponent<comp::Light>();
		lightComp->values.color = SA_COLOR_WHITE;
		lightComp->values.position = transform->position + sa::Vector3(3, 2, -3);
		lightComp->values.strength = 10.0f;

		return entity;
	}

	void TestLayer::onAttach(Engine& engine, RenderWindow& window) {
		m_pEngine = &engine;
		m_pWindow = &window;

		//createModelEntity(engine, "resources/models/Box.gltf");

		/*
		for (int i = 0; i < 30; i++) {
			createModelEntity(engine, "resources/models/adamHead/adamHead.gltf");
			createModelEntity(engine, "resources/models/lieutenantHead/lieutenantHead.gltf");
			createModelEntity(engine, "resources/models/Suzanne.dae");
		}
		*/

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