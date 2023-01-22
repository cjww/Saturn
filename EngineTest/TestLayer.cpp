
#include "TestLayer.h"
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
		lightComp->values.intensity = 10.0f;

		return entity;
	}

	void TestLayer::onAttach(Engine& engine, RenderWindow& window) {
		m_pEngine = &engine;
		m_pWindow = &window;

		m_camera.setViewport(Rect{ { 0, 0 }, window.getCurrentExtent() });
		m_camera.setPosition({ 0, 0, -6 });
		m_camera.lookAt({ 0, 0, 0 });
		
		m_colorTexture = DynamicTexture2D(TextureTypeFlagBits::COLOR_ATTACHMENT | TextureTypeFlagBits::SAMPLED, window.getCurrentExtent());
		
		m_renderTarget.framebuffer = m_pEngine->getRenderPipeline().getRenderTechnique()->createColorFramebuffer(m_colorTexture);

		
		m_pEngine->on<engine_event::OnRender>([&](engine_event::OnRender& e, Engine& engine) {
			e.pRenderPipeline->render(&m_camera, &m_renderTarget);
			//m_colorTexture.swap();
			sa::Renderer::get().swapFramebuffer(m_renderTarget.framebuffer);
		});

		m_pEngine->on<engine_event::WindowResized>([&](engine_event::WindowResized& e, Engine& engine) {
			m_colorTexture.destroy();
			m_colorTexture = DynamicTexture2D(TextureTypeFlagBits::COLOR_ATTACHMENT | TextureTypeFlagBits::SAMPLED, e.newExtent);
			sa::Renderer::get().destroyFramebuffer(m_renderTarget.framebuffer);
			m_renderTarget.framebuffer = engine.getRenderPipeline().getRenderTechnique()->createColorFramebuffer(m_colorTexture);
			m_renderTarget.bloomData.isInitialized = false;
		});



		Entity light = m_pEngine->getCurrentScene()->createEntity();
		LightData& values = light.addComponent<comp::Light>()->values;
		values.position = { 0, 0, -2 };
		values.intensity = 2.0f;
		values.color = Color{ 0.5, 1.0f, 0.5f, 1.0f };

		
	}

	void TestLayer::onUpdate(float dt) {
		static float timer = 0.0f;
		static bool spawned = false;
		timer += dt;
		static Entity box;
		if (timer > 1 && !spawned) {
			spawned = true;
			box = m_pEngine->getCurrentScene()->createEntity();

			box.addComponent<comp::Transform>()->position = {0, 0, 0};
			box.addComponent<comp::Model>()->modelID = AssetManager::get().loadBox();

		}
		
		if (spawned) {
			//box.getComponent<comp::Transform>()->position = glm::vec3(1, 0, 0) * sin(timer * 4) * 2.f;
		}

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
		
		if (ImGui::Begin("w")) {
			if (m_renderTarget.bloomData.isInitialized) {
				ImGui::Image(m_renderTarget.outputTexture, ImVec2(200, 200));
			}

			ImGui::End();
		}
	}
}