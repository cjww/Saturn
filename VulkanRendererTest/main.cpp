#define DEBUG_LOG

#include <iostream>

#include <RenderWindow.hpp>
#include <Renderer.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include <chrono>

class CameraController {
private:
	RenderWindow& window;
	glm::vec3 viewForward;
	glm::vec3 viewPos;
	glm::vec2 lastMousePos;

	glm::mat4 mat = glm::mat4(1);
	bool escapePressed;

public:
	float speed = 10.f;
	float sensitivty = 1.f;
		
	bool mouseLocked;

	CameraController(RenderWindow& window) : window(window) {
		viewForward = glm::vec3(0, 0, 1);
		viewPos = glm::vec3(0.f);
		
		mouseLocked = true;
		window.setHideCursor(true);
	};

	glm::mat4 getView(float dt) {
		int hori = glfwGetKey(window.getWindowHandle(), GLFW_KEY_D) - glfwGetKey(window.getWindowHandle(), GLFW_KEY_A);
		int vert = glfwGetKey(window.getWindowHandle(), GLFW_KEY_W) - glfwGetKey(window.getWindowHandle(), GLFW_KEY_S);

		glm::vec2 mPos = window.getCursorPosition();
		glm::vec2 center = glm::vec2(window.getCurrentExtent().x / 2, window.getCurrentExtent().y / 2);
		glm::vec2 diff = mPos - center;
		if (mouseLocked) {
			window.setCursorPosition(center);
		}
		else {
			diff = glm::vec2(0, 0);
		}

		if (window.getKey(GLFW_KEY_ESCAPE) && !escapePressed) {
			mouseLocked = !mouseLocked;
			window.setHideCursor(mouseLocked);
		}
		escapePressed = window.getKey(GLFW_KEY_ESCAPE);

		int up = glfwGetKey(window.getWindowHandle(), GLFW_KEY_SPACE) - glfwGetKey(window.getWindowHandle(), GLFW_KEY_LEFT_CONTROL);
		bool sprint = glfwGetKey(window.getWindowHandle(), GLFW_KEY_LEFT_SHIFT);

		viewForward = glm::vec3(glm::vec4(viewForward, 0.0f) * glm::rotate(diff.x * dt * sensitivty, glm::vec3(0, 1, 0)));

		glm::vec3 right = glm::cross(viewForward, glm::vec3(0, 1, 0));
		viewForward = glm::vec3(glm::vec4(viewForward, 0.0f) * glm::rotate(diff.y * dt * sensitivty, right));


		float finalSpeed = speed;
		if (sprint) {
			finalSpeed *= 2;
		}

		viewPos += right * (float)hori * dt * finalSpeed;
		viewPos += viewForward * (float)vert * dt * finalSpeed;
		viewPos += glm::vec3(0, 1, 0) * (float)up * dt * finalSpeed;

		
		return glm::lookAt(viewPos, viewPos + viewForward, glm::vec3(0, 1, 0));
	}

	glm::mat4 getViewRayTracing(float dt) {
		int hori = glfwGetKey(window.getWindowHandle(), GLFW_KEY_D) - glfwGetKey(window.getWindowHandle(), GLFW_KEY_A);
		int vert = glfwGetKey(window.getWindowHandle(), GLFW_KEY_W) - glfwGetKey(window.getWindowHandle(), GLFW_KEY_S);

		glm::vec2 mPos = window.getCursorPosition();
		glm::vec2 center = glm::vec2(window.getCurrentExtent().x / 2, window.getCurrentExtent().y / 2);
		glm::vec2 diff = mPos - center;
		if (mouseLocked) {
			window.setCursorPosition(center);
		}
		else {
			diff = glm::vec2(0, 0);
		}

		if (window.getKey(GLFW_KEY_ESCAPE) && !escapePressed) {
			mouseLocked = !mouseLocked;
			window.setHideCursor(mouseLocked);
		}
		escapePressed = window.getKey(GLFW_KEY_ESCAPE);
		
		int up = glfwGetKey(window.getWindowHandle(), GLFW_KEY_SPACE) - glfwGetKey(window.getWindowHandle(), GLFW_KEY_LEFT_CONTROL);
		bool sprint = glfwGetKey(window.getWindowHandle(), GLFW_KEY_LEFT_SHIFT);

		mat *= glm::rotate(diff.x * dt * sensitivty, glm::vec3(0, 1, 0));

		glm::vec3 right = glm::cross(viewForward, glm::vec3(0, 1, 0));
		mat *= glm::rotate(-diff.y * dt * sensitivty, right);


		float finalSpeed = speed;
		if (sprint) {
			finalSpeed *= 2;
		}

		mat = glm::translate(mat, -right * (float)hori * dt * finalSpeed);
		mat = glm::translate(mat, viewForward * (float)vert * dt * finalSpeed);
		mat = glm::translate(mat, glm::vec3(0, 1, 0) * (float)up * dt * finalSpeed);

		return mat;

	}

	glm::mat4 getProjection(float fovDegrees) {
		auto projection = glm::perspective(glm::radians(fovDegrees), (float)window.getCurrentExtent().x / window.getCurrentExtent().y, 0.01f, 1000.0f);
		projection[1][1] *= -1;
		return projection;
	}
};

class FrameTimer {
private:
	std::chrono::steady_clock::time_point m_lastTime;
public:
	FrameTimer() {
		m_lastTime = std::chrono::high_resolution_clock::now();
	}
	// return time since last call in seconds
	float getDeltaTime() {
		// Time calculations
		auto now = std::chrono::high_resolution_clock::now();
		auto dtDuration = std::chrono::duration<float, std::ratio<1, 1>>(now - m_lastTime);
		m_lastTime = now;
		float dt = dtDuration.count();
		return dt;
	}
};

struct RenderPass {
	vr::Texture* depthImage;
	uint32_t renderPass;
	uint32_t frameBuffer;
};

struct Vertex {
	glm::vec4 position;
};

struct VertexUV {
	glm::vec4 position;
	glm::vec2 uv;
};

struct Particle {
	glm::mat4 mat;
};

struct UBO {
	glm::mat4 view;
	glm::mat4 projection;
	int matrixCount;
};

struct ConfigBuffer {
	glm::mat4 transform;
	int matrixCount;
};

struct SceneUbo {
	glm::mat4 view;
	glm::mat4 proj;
};

struct ObjectUbo {
	glm::mat4 model;
};


Vertex box[8] = {
	// forward
	{ { -1.0f, 1.0f, 1.0f, 1.0f } },
	{ { 1.0f, 1.0f, 1.0f, 1.0f } },
	{ { -1.0f, -1.0f, 1.0f, 1.0f } },
	{ { 1.0f, -1.0f, 1.0f, 1.0f } },
	// back
	{ { -1.0f, 1.0f, -1.0f, 1.0f } },
	{ { 1.0f, 1.0f, -1.0f, 1.0f } },
	{ { -1.0f, -1.0f, -1.0f, 1.0f } },
	{ { 1.0f, -1.0f, -1.0f, 1.0f } }
};

uint32_t boxIndices[] = {
	// Forward
	0, 1, 2, 1, 3, 2,
	// Right
	1, 5, 3, 5, 7, 3,
	// Back
	5, 4, 7, 4, 6, 7,
	// Left
	4, 0, 6, 0, 2, 6,
	// Top
	4, 5, 0, 5, 1, 0,
	// Bottom
	7, 6, 3, 6, 2, 3
};

VertexUV quad[] = {
	{ glm::vec4(-0.5f, 0.5f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f) },
	{ glm::vec4(0.5f, 0.5f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f) },
	{ glm::vec4(0.5f, -0.5f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f) },
	{ glm::vec4(-0.5f, -0.5f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f) }
};

uint32_t quadIndices[] = {
	0, 1, 3,
	1, 2, 3
};

void computeTest(RenderWindow& window);
void textureTest(RenderWindow& window);
void texture3DTest(RenderWindow& window);

RenderPass createSimpleRenderPass(RenderWindow& window);


int main() {

#ifdef _WIN32
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	try {
		const int WIDTH = 1000, HEIGHT = 600;
		
		RenderWindow window(WIDTH, HEIGHT, "Hello vulkan");
		vr::Renderer::init(&window);
		
		//computeTest(window);
		//textureTest(window);
		texture3DTest(window);
	}
	catch (std::exception e) {
		std::cout << e.what() << std::endl;
	}

	vr::Renderer::cleanup();

	return 0;
}


void textureTest(RenderWindow& window) {

	RenderPass renderPass = createSimpleRenderPass(window);
	vr::Renderer* renderer = vr::Renderer::get();

	vr::ShaderPtr vertexShader = renderer->createShader("TextureVertexShader.spv", VK_SHADER_STAGE_VERTEX_BIT);
	vr::ShaderPtr fragmentShader = renderer->createShader("TextureFragmentShader.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
	vr::ShaderSetPtr shaderSet = renderer->createShaderSet(vertexShader, fragmentShader);

	uint32_t pipeline = renderer->createPipeline(shaderSet, renderPass.renderPass, 0);

	CameraController controller(window);
	controller.sensitivty = 5.f;
	controller.speed = 10.f;

	vr::DescriptorSetPtr descriptorSet = shaderSet->getDescriptorSet(0);
	SceneUbo sceneUbo = {};
	sceneUbo.view = controller.getView(0.0f);
	sceneUbo.proj = controller.getProjection(60.f);

	vr::Buffer* sceneUniformBuffer = renderer->createUniformBuffer(sizeof(SceneUbo), &sceneUbo);
	renderer->updateDescriptorSet(descriptorSet, 0, sceneUniformBuffer, nullptr, nullptr, true);

	ObjectUbo objectUbo = {};
	objectUbo.model = glm::mat4(1.0f);

	vr::Buffer* objectUniformBuffer = renderer->createUniformBuffer(sizeof(ObjectUbo), &objectUbo);
	renderer->updateDescriptorSet(descriptorSet, 1, objectUniformBuffer, nullptr, nullptr, true);

	vr::Buffer* vertexBuffer = renderer->createVertexBuffer(sizeof(quad), quad);
	vr::Buffer* indexBuffer = renderer->createIndexBuffer(sizeof(quadIndices), quadIndices);

	vr::SamplerPtr sampler = renderer->createSampler(VK_FILTER_NEAREST);
	
	vr::Image image("Box.png");
	vr::Texture* texture = renderer->createTexture2D(renderPass.frameBuffer, renderPass.renderPass, 0, image);
	renderer->updateDescriptorSet(descriptorSet, 2, nullptr, texture, sampler, true);

	FrameTimer timer;
	float time = 0.0f;
	int frames = 0;
	while (window.isOpen()) {
		window.pollEvents();

		float dt = timer.getDeltaTime();
		time += dt;
		frames++;
		if (time >= 0.5f) {
			float cpuTime = time / frames;
			window.setWindowTitle("Average CPU time " + std::to_string(cpuTime) + ", FPS: " + std::to_string((int)(1 / cpuTime)));
			time = 0.0f;
			frames = 0;
		}

		if (renderer->beginFrame()) {
			sceneUbo.view = controller.getView(dt);
			memcpy(sceneUniformBuffer->mappedData, &sceneUbo, sizeof(sceneUbo));
			renderer->updateDescriptorSet(descriptorSet, 0, sceneUniformBuffer, nullptr, nullptr, false);


			renderer->beginRenderPass(renderPass.renderPass, renderPass.frameBuffer, VK_SUBPASS_CONTENTS_INLINE);
		
			renderer->bindPipeline(pipeline);

			renderer->bindVertexBuffer(vertexBuffer);
			renderer->bindIndexBuffer(indexBuffer);

			renderer->bindDescriptorSet(descriptorSet, pipeline);
			//renderer->bindViewport();

			renderer->drawIndexed(6, 1);

			renderer->endRenderPass();

			renderer->endFrame();
			renderer->present();
		}
		
	}

}

void computeTest(RenderWindow& window) {
	
	RenderPass renderPass = createSimpleRenderPass(window);
	vr::Renderer* renderer = vr::Renderer::get();

	vr::ShaderPtr vshader = renderer->createShader("ComputeVertexShader.spv", VK_SHADER_STAGE_VERTEX_BIT);
	vr::ShaderPtr fshader = renderer->createShader("ComputeFragmentShader.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
	vr::ShaderSetPtr shaderSet = renderer->createShaderSet(vshader, fshader);

	auto pipeline = renderer->createPipeline(shaderSet, renderPass.renderPass, 0);

	// BOX
	vr::Buffer* boxVertexbuffer = renderer->createVertexBuffer(sizeof(box), box);
	vr::Buffer* boxIndexBuffer = renderer->createIndexBuffer(sizeof(boxIndices), boxIndices);

	// Compute Shader
	const int count = 2048 * 2;
	vr::ShaderPtr computeShader = renderer->createShader("ComputeShader.spv", VK_SHADER_STAGE_COMPUTE_BIT);
	vr::ShaderSetPtr computeSet = renderer->createShaderSet(computeShader);
	auto computePipeline = renderer->createPipeline(computeSet);
	vr::DescriptorSetPtr computeDescSet = computeSet->getDescriptorSet(0);

	// Compute Shader uniform buffer
	int ccount = count;
	vr::Buffer* configBuffer = renderer->createUniformBuffer(sizeof(ccount), &ccount);
	renderer->updateDescriptorSet(computeDescSet, 0, configBuffer, nullptr, nullptr, true);

	// Compute ouputBuffer
	vr::Buffer* outputBuffer = renderer->createStorageBuffer(sizeof(glm::mat4) * count, nullptr);
	renderer->updateDescriptorSet(computeDescSet, 2, outputBuffer, nullptr, nullptr, true);


	float time = 1.0f;

	// Compute Command buffer
	vr::CommandBufferPtr computeCommandBuffer = renderer->createCommandBuffer(true);
	renderer->recordCommandBuffer(computeCommandBuffer, [&](uint32_t frameIndex) {
		renderer->bindPipeline(computePipeline, computeCommandBuffer, frameIndex);
		renderer->bindDescriptorSet(computeDescSet, computePipeline, computeCommandBuffer, frameIndex);
		renderer->pushConstants(computePipeline, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(float), &time, computeCommandBuffer, frameIndex);
		vkCmdDispatch(computeCommandBuffer->buffers[frameIndex], (count / 256) + 1, 1, 1);
		});

	// Dispatch compute shader
	auto fence = renderer->submitToComputeQueue(computeCommandBuffer);

	// Vertex Shader uniformBuffer
	CameraController controller(window);
	vr::DescriptorSetPtr descriptorSet = shaderSet->getDescriptorSet(0);
	UBO ubo = {};
	ubo.matrixCount = count;
	ubo.view = controller.getView(0.0f);
	ubo.projection = controller.getProjection(60.f);

	vr::Buffer* uniformBuffer = renderer->createUniformBuffer(sizeof(ubo), &ubo);

	renderer->updateDescriptorSet(descriptorSet, 0, uniformBuffer, nullptr, nullptr, true);

	// Wait until compute shader is done
	renderer->waitForFence(fence);
	// update vertex storage buffer with compute shader output
	renderer->updateDescriptorSet(descriptorSet, 1, outputBuffer, nullptr, nullptr, true);

	// main loop
	auto lastTime = std::chrono::high_resolution_clock::now();
	while (window.isOpen()) {
		window.pollEvents();

		// Time calculations
		auto now = std::chrono::high_resolution_clock::now();
		auto dtDuration = std::chrono::duration<float, std::ratio<1, 1>>(now - lastTime);
		lastTime = now;
		float dt = dtDuration.count();
		time += dt;
		//window.setWindowTitle("FPS: " + std::to_string(1 / dt));


		if (renderer->beginFrame()) {
			/*
			*/
			// Rerecord compute command buffer with new time
			renderer->recordCommandBuffer(computeCommandBuffer, [&](uint32_t frameIndex) {
				renderer->bindPipeline(computePipeline, computeCommandBuffer, frameIndex);
				renderer->bindDescriptorSet(computeDescSet, computePipeline, computeCommandBuffer, frameIndex);
				renderer->pushConstants(computePipeline, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(float), &time, computeCommandBuffer, frameIndex);
				vkCmdDispatch(computeCommandBuffer->buffers[frameIndex], (count / 256) + 1, 1, 1);
			}, true);
			// Dispatch compute shader
			fence = renderer->submitToComputeQueue(computeCommandBuffer);

			// update vertex uniform buffer
			ubo.view = controller.getView(dt);
			memcpy((char*)uniformBuffer->mappedData + offsetof(UBO, UBO::view), &ubo.view, sizeof(glm::mat4));
			renderer->updateDescriptorSet(descriptorSet, 0, uniformBuffer, nullptr, nullptr, false);
			/*
			*/
			// wait for compute shader to finish
			renderer->waitForFence(fence);
			// update vertexShader with compute output
			renderer->updateDescriptorSet(descriptorSet, 1, outputBuffer, nullptr, nullptr, false);

			// draw frame
			renderer->beginRenderPass(renderPass.renderPass, renderPass.frameBuffer, VK_SUBPASS_CONTENTS_INLINE, glm::vec3(1.0f, 0.0f, 0.0f));

			renderer->bindPipeline(pipeline);
		
			renderer->bindVertexBuffer(boxVertexbuffer);
			renderer->bindIndexBuffer(boxIndexBuffer);

			renderer->bindDescriptorSet(descriptorSet, pipeline);

			renderer->drawIndexed(sizeof(boxIndices) / sizeof(uint32_t), count);

			renderer->endRenderPass();
		}

		renderer->endFrame();

		renderer->present();

	}
}

void texture3DTest(RenderWindow& window) {
	RenderPass renderPass = createSimpleRenderPass(window);
	vr::Renderer* renderer = vr::Renderer::get();

	vr::ShaderPtr vertexShader = renderer->createShader("RaytracingVertex.spv", VK_SHADER_STAGE_VERTEX_BIT);
	vr::ShaderPtr fragmentShader = renderer->createShader("RaytracingFragment.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
	vr::ShaderSetPtr shaderSet = renderer->createShaderSet(vertexShader, fragmentShader);

	auto pipeline = renderer->createPipeline(shaderSet, renderPass.renderPass, 0);

	auto descriptorSet = shaderSet->getDescriptorSet(0);


	/*vr::Image image("Box.png");
	vr::Texture* tex = renderer->createTexture3D({ image.getExtent().width, image.getExtent().height, 1 }, VK_FORMAT_R8_UNORM);
	
	std::vector<uint8_t> data(image.getWidth() * image.getHeight());
	for(int i = 0; i < data.size(); i++) {
		data[i] = image.getPixels()[i * image.getChannelCount()];
	}*/

	vr::Texture* tex = renderer->createTexture3D({ 32, 32, 32 }, VK_FORMAT_R8_UNORM);

	std::vector<uint8_t> data(32 * 32 * 32);
	for (int i = 0; i < data.size(); i++) {
		data[i] = i;
	}

	renderer->updateTexture(tex, renderPass.frameBuffer, renderPass.renderPass, 0, data.data(), data.size());
	
	vr::SamplerPtr sampler = renderer->createSampler(VK_FILTER_NEAREST);

	renderer->updateDescriptorSet(descriptorSet, 0, nullptr, tex, sampler, true);
	
	auto variablesDescSet = shaderSet->getDescriptorSet(1);

	CameraController cameraController(window);

	struct Variables {
		alignas(16) glm::mat4 worldMat;
		alignas(16) glm::vec3 windowSize;
		alignas(4) float time = 0.0f;
	} variables;
	variables.windowSize = glm::vec3(window.getCurrentExtent(), 0);
	variables.worldMat = glm::mat4(1);

	auto buffer = renderer->createUniformBuffer(sizeof(Variables), &variables);
	
	renderer->initImGUI(renderPass.renderPass, 0);

	FrameTimer frameTimer;
	bool keyPressed = false;
	while (window.isOpen()) {
		window.pollEvents();

		if (window.getKey(GLFW_KEY_R) && !keyPressed)
		{
			fragmentShader->load("RaytracingFragment.spv");
			shaderSet = renderer->createShaderSet(vertexShader, fragmentShader);
			pipeline = renderer->createPipeline(shaderSet, renderPass.renderPass, 0);
			descriptorSet = shaderSet->getDescriptorSet(0);
			renderer->updateDescriptorSet(descriptorSet, 0, nullptr, tex, sampler, true);

			variablesDescSet = shaderSet->getDescriptorSet(1);

			DEBUG_LOG_INFO("Reloaded shader");
		}
		keyPressed = window.getKey(GLFW_KEY_R);

		if(renderer->beginFrame()) {
			float dt = frameTimer.getDeltaTime();
			variables.time += dt;
			variables.worldMat = cameraController.getViewRayTracing(dt);

			memcpy(buffer->mappedData, &variables, sizeof(variables));
			renderer->updateDescriptorSet(variablesDescSet, 0, buffer, nullptr, nullptr, false);
			renderer->beginRenderPass(renderPass.renderPass, renderPass.frameBuffer, VK_SUBPASS_CONTENTS_INLINE);
			
			renderer->bindPipeline(pipeline);
			renderer->bindDescriptorSet(descriptorSet, pipeline);
			renderer->bindDescriptorSet(variablesDescSet, pipeline);
			renderer->draw(6, 1);

			renderer->newFrameImGUI();

			ImGui::ShowDemoWindow();

			renderer->endFrameImGUI();
			
			renderer->endRenderPass();
			renderer->endFrame();

			renderer->present();
		}

	}
	renderer->cleanupImGUI();


}

RenderPass createSimpleRenderPass(RenderWindow& window) {
	RenderPass renderPassStruct = {};

	vr::Renderer* renderer = vr::Renderer::get();

	renderPassStruct.depthImage = renderer->createDepthTexture({ (uint32_t)window.getCurrentExtent().x, (uint32_t)window.getCurrentExtent().y });

	std::vector<VkAttachmentDescription> attachments(2);
	attachments[0] = renderer->getSwapchainAttachment();
	attachments[1] = vr::getDepthAttachment(renderPassStruct.depthImage->format, renderPassStruct.depthImage->sampleCount);

	std::vector<VkAttachmentReference> refrences(2);
	refrences[0].attachment = 0;
	refrences[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	refrences[1].attachment = 1;
	refrences[1].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	std::vector<VkSubpassDescription> subpasses(1);
	subpasses[0].flags = 0;
	subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpasses[0].inputAttachmentCount = 0;
	subpasses[0].preserveAttachmentCount = 0;

	subpasses[0].colorAttachmentCount = 1;
	subpasses[0].pColorAttachments = &refrences[0];
	subpasses[0].pDepthStencilAttachment = &refrences[1];

	renderPassStruct.renderPass = renderer->createRenderPass(attachments, subpasses, { });

	renderPassStruct.frameBuffer = renderer->createSwapchainFramebuffer(renderPassStruct.renderPass, { renderPassStruct.depthImage });

	return std::move(renderPassStruct);
}
