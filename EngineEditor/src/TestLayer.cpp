
#include "TestLayer.h"

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

		comp::Light* lightComp = light.addComponent<comp::Light>();
		lightComp->values.color = color;
		lightComp->values.position = glm::vec4(position, lightComp->values.position.w);
		return light;
	}

	Entity TestLayer::createModelEntity(Engine& engine, const std::filesystem::path& modelPath, float scale) {
		Entity entity = engine.getCurrentScene()->createEntity(modelPath.filename().generic_string());

		comp::Model* model = entity.addComponent<comp::Model>();

		m_completions.insert({ entity, sa::AssetManager::get().importAsset<ModelAsset>(modelPath) });
		
		//m_completions.at(entity).wait();

		if (m_completions.at(entity)->getProgress().isDone()) {
			//entity.getComponent<comp::Model>()->modelID = m_completions.at(entity);
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
		
		return entity;
	}

	Entity TestLayer::createDynamicBox(const Vector3& position, float scale) {
		Entity box = m_pEngine->getCurrentScene()->createEntity("Box");
		box.addComponent<comp::Transform>()->position = position;
		box.addComponent<comp::RigidBody>(false);
		box.addComponent<comp::BoxCollider>(Vector3(scale, scale, scale));
		box.addComponent<comp::Model>()->modelID = AssetManager::get().loadCube()->getID();

		return box;
	}

	void TestLayer::onAttach(Engine& engine, RenderWindow& window) {
		m_pEngine = &engine;
		m_pWindow = &window;

		//createModelEntity(engine, "resources/models/Box.gltf");

		
		/*
		Entity groundEntity = engine.getCurrentScene()->createEntity("Ground");
		groundEntity.addComponent<comp::Model>()->modelID = AssetManager::get().loadQuad();
		comp::Transform* transform = groundEntity.addComponent<comp::Transform>();
		transform->scale = { 500, 500, 1 };
		transform->rotation = glm::rotate(transform->rotation, glm::radians(-90.f), { 1, 0, 0 });
		transform->position.y -= 1;
		groundEntity.addComponent<comp::BoxCollider>(Vector3(100, 100, 0.1f));

		Entity adam = createModelEntity(engine, "resources/models/adamHead/adamHead.gltf");
		Entity entity = createModelEntity(engine, "resources/models/sponza/scene.gltf");
		entity.removeComponent<comp::Script>();

		createLight(engine, Vector3(2.f, 2.f, -3.f));

		Entity parent = engine.getCurrentScene()->createEntity("Lights");

		for (int i = 0; i < 500; i++) {
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

			Entity l = createLight(engine, pos, colors[rand() % 7]);
			l.setParent(parent);
		}

		//createModelEntity(engine, "resources/models/stg-44/source/wpn_h1_asl_mp44/wpn_h1_asl_mp44.obj", 0.1f);
		//createModelEntity(engine, "resources/models/sci-fi_girl/scene.gltf");
		//createModelEntity(engine, "resources/models/cyberpunk_speeder/scene.gltf");


		/*
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
		*/

		/*
		adam.addScript("Movable.lua");
		adam.addScript("Inventory.lua");
		engine.createSystemScript("test.lua");
		engine.createSystemScript("test2.lua");
		engine.getCurrentScene()->addScript(adam, "Movable.lua");
		engine.getCurrentScene()->addScript(adam, "Inventory.lua");
		adam.addScript("Movable.lua");
		adam.addScript("Inventory.lua");

		Entity box = m_pEngine->getCurrentScene()->createEntity("Box");
		box.addComponent<comp::Transform>()->position = glm::vec3(0, 10, 0);
		box.addComponent<comp::RigidBody>(false);
		box.addComponent<comp::BoxCollider>(glm::vec3(1, 1, 1));
		*/

	}

	void TestLayer::onUpdate(float dt) {
		SA_PROFILE_FUNCTION();
		

	}

	void TestLayer::onImGuiRender() {
		if(ImGui::Begin("Test window")) {
			std::queue<Entity> entitiesDone;
			std::set<ModelAsset*> progressSet;
			for (const auto& [entity, progress] : m_completions) {
				if (entity.isNull()) {
					entitiesDone.push(entity);
					continue;
				}
				size_t size = progressSet.size();
				progressSet.insert(progress);
				if (size != progressSet.size()) {
					ImGui::Text("%s", entity.getComponent<comp::Name>()->name.c_str());
					ImGui::SameLine();
					ImGui::ProgressBar(progress->getProgress().getCompletion());
				}

				if (progress->getProgress().isDone()) {
					//entity.getComponent<comp::Model>()->modelID = progress;
					entitiesDone.push(entity);
				}
			}

			while (!entitiesDone.empty()) {
				m_completions.erase(entitiesDone.front());
				entitiesDone.pop();
			}

		}
		ImGui::End();
	}
}