
#include "TestLayer.h"
#include "Tools\Profiler.h"

namespace sa {

	int TestLayer::randomRange(int min, int max) {
		return (rand() % (max - min)) - (max - min) / 2;
	}

	
	void TestLayer::onAttach(Engine& engine, RenderWindow& window) {
		m_pEngine = &engine;
		m_pWindow = &window;
		

		Scene* pScene = sa::AssetManager::get().createAsset<Scene>("MyScene");
		
		Entity cameraEntity = pScene->createEntity("MainCamera");
		comp::Transform* cameraTransform = cameraEntity.addComponent<comp::Transform>();
		comp::Camera* camera = cameraEntity.addComponent<comp::Camera>();
		cameraTransform->position = { 0, 1, -5 };

		Entity box = pScene->createEntity("Box");
		box.addComponent<comp::Transform>();
		box.addComponent<comp::Model>()->modelID = sa::AssetManager::get().loadCube()->getID();

		Entity light = pScene->createEntity("Light");
		comp::Transform* lightTransform = light.addComponent<comp::Transform>();
		lightTransform->rotation = glm::rotate(lightTransform->rotation, glm::radians(-45.f), glm::vec3(1, 0, 0));

		comp::Light* lightComp = light.addComponent<comp::Light>();
		lightComp->values.type = sa::LightType::DIRECTIONAL;

		//lightComp->values.direction = glm::vec4(glm::normalize(glm::vec3(0.f, -1.f, 1.f)), 0.f);

		box.addScript("test2.lua");
		light.addScript("test2.lua");


		m_pEngine->setScene(pScene);
	}

	void TestLayer::onUpdate(float dt) {
		m_pEngine->getCurrentScene()->runtimeUpdate(dt);
	}

	void TestLayer::onImGuiRender() {
		
	}
}