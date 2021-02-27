#include "RenderWindow.hpp"
#include "Camera.hpp"

#include <chrono>

int main() {

	vr::RenderWindow window(1000, 600, "Minecraft");
	vr::Renderer* renderer = vr::Renderer::get();
	vr::ShaderPtr chunkVertexShader = renderer->createShader("ChunkVertexShader.spv", VK_SHADER_STAGE_VERTEX_BIT);
	vr::ShaderPtr chunkFragmentShader = renderer->createShader("ChunkFragmentShader.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
	vr::ShaderSet chunkShaderSet = renderer->createShaderSet(chunkVertexShader, chunkFragmentShader);

	vr::DescriptorSetPtr sceneDescriptorSet = chunkShaderSet.getDescriptorSet(0);
	Camera camera(window, sceneDescriptorSet, 0);

	auto startTime = std::chrono::high_resolution_clock::now();

	while (window.isOpen()) {
		window.pollEvents();

		window.beginFrame();

		auto now = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration<float>(now - startTime);
		float dt = duration.count();

		camera.update(dt);

		window.endFrame();


	}

	return 0;
}