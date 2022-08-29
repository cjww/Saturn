
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

		m_completions[entity] = sa::AssetManager::get().loadModelAsync(modelPath);

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
		m_camera.setViewport(Rect{ { 0, 0 }, window.getCurrentExtent() });
		m_camera.setPosition({ 0, 0, -10 });
		m_camera.lookAt({ 2, 0, 0 });
		m_pCameraController = std::make_unique<CameraController>(window, m_camera);

		engine.getCurrentScene()->addActiveCamera(&m_camera);


		createModelEntity(engine, "models/adamHead/adamHead.gltf");
		createModelEntity(engine, "models/lieutenantHead/lieutenantHead.gltf");
		createModelEntity(engine, "models/steampunk_underwater_explorer/scene.gltf", 0.5f);
		createModelEntity(engine, "models/steampunk_glasses__goggles/scene.gltf");
		createModelEntity(engine, "models/viking_room/scene.gltf", 0.2f);

		engine.createSystemScript("test.lua");



	}

	void TestLayer::onUpdate(float dt) {
		SA_PROFILE_FUNCTION();
		m_pCameraController->update(dt);

		if (m_infoClock.getElapsedTime() > 0.5f) {
			m_infoClock.restart();

			for (const auto& [entity, progress] : m_completions) {
				std::cout << "Entity " << entity.getComponent<comp::Name>()->name << " : " << progress.getProgress() * 100 << "%" << std::endl;
				if (progress.isDone()) {
					entity.getComponent<comp::Model>()->modelID = progress.get();
					m_completions.erase(entity);
					break;
				}
			}
		}

	}
}