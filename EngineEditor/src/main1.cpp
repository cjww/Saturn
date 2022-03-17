/*
#include "EngineEditor.h"
int main(int argc, char** args) {

	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);


	std::string projectPath;
	if (argc > 1) {
		projectPath = args[1];
	}
	else {
		//Create project

	}

	EngineEditor editor;

	try {
		editor.openProject(projectPath);
	}
	catch (const std::exception& e) {
		std::cout << e.what() << std::endl;
	}
	return 0;
}
*/
#include <crtdbg.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include <stdio.h>          // printf, fprintf
#include <stdlib.h>         // abort
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <Engine.h>
#include <Graphics/RenderWindow.h>
#include <Tools\Clock.h>

#include <SPIRV\spirv_cross.hpp>

#include <Graphics\Vulkan\Renderer.hpp>

int main(int argc, char** args) {

	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	sa::Engine engine;
	sa::RenderWindow window(1200, 800, "test");
	
	engine.setup(&window);
	sa::Camera* camera = engine.getCurrentScene()->newCamera(&window);
	camera->setPosition({ 0, 0, 1 });
	camera->lookAt({ 0, 0, 0 });

	engine.getCurrentScene()->addActiveCamera(camera);

	sa::Entity entity = engine.getCurrentScene()->createEntity();
	entity.addComponent<comp::Transform>();
	entity.addComponent<comp::Model>()->modelID = sa::ResourceManager::get().loadQuad();

	engine.getCurrentScene()->on<sa::event::UpdatedScene>([&](sa::event::UpdatedScene e, sa::Scene& scene) {
		entity.getComponent<comp::Transform>()->position.z -= e.dt;
	});

	sa::Clock clock;

	sa::Texture texture = engine.getRenderTechnique()->getOutputTexture();
		
	auto sampler = vr::Renderer::get().createSampler(VK_FILTER_LINEAR);

	while (window.isOpen()) {
		window.pollEvents();

		
		engine.update(clock.restart());


		//ImGui::Image(vr::Renderer::get().getImTextureID(texture, sampler), ImVec2(500, 300));
		

		engine.draw();
	}

	engine.cleanup();
	return 0;
}