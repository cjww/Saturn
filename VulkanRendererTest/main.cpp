#include <iostream>

#include <RenderWindow.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include <chrono>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

class CameraController {
private:
	vr::RenderWindow& window;
	glm::vec3 viewForward;
	glm::vec3 viewPos;
	glm::vec2 lastMousePos;

public:
	float speed = 10.f;
	float sensitivty = 1.f;
		
	bool mouseLocked;

	CameraController(vr::RenderWindow& window) : window(window) {
		viewForward = glm::vec3(0, 0, 1);
		viewPos = glm::vec3(0.f);
		
		mouseLocked = true;
		window.setHideCursor(true);
	};

	glm::mat4 getView(float dt) {
		int hori = glfwGetKey(window.getWindowHandle(), GLFW_KEY_D) - glfwGetKey(window.getWindowHandle(), GLFW_KEY_A);
		int vert = glfwGetKey(window.getWindowHandle(), GLFW_KEY_W) - glfwGetKey(window.getWindowHandle(), GLFW_KEY_S);

		glm::vec2 mPos = window.getCursorPosition();
		glm::vec2 center = glm::vec2(window.getCurrentExtent().width / 2, window.getCurrentExtent().height / 2);
		glm::vec2 diff = mPos - center;
		if (mouseLocked) {
			window.setCursorPosition(center);
		}
		else {
			diff = glm::vec2(0, 0);
		}

		if (glfwGetKey(window.getWindowHandle(), GLFW_KEY_ESCAPE)) {
			mouseLocked = false;
			window.setHideCursor(false);
		}

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

	glm::mat4 getProjection(float fovDegrees) {
		auto projection = glm::perspective(glm::radians(fovDegrees), (float)window.getCurrentExtent().width / window.getCurrentExtent().height, 0.01f, 1000.0f);
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
	vr::ImagePtr depthImage;
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
	{ glm::vec4(-0.5f, 0.5f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f) },
	{ glm::vec4(0.5f, 0.5f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f) },
	{ glm::vec4(0.5f, -0.5f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f) },
	{ glm::vec4(-0.5f, -0.5f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f) }
};

uint32_t quadIndices[] = {
	0, 1, 3,
	1, 2, 3
};

void computeTest(vr::RenderWindow& window);
void textureTest(vr::RenderWindow& window);

RenderPass createSimpleRenderPass(vr::RenderWindow& window);

int main() {

	try {
		const int WIDTH = 1000, HEIGHT = 600;
		vr::RenderWindow window(WIDTH, HEIGHT, "Hello vulkan");


		//computeTest(window);
		textureTest(window);
	}
	catch (std::exception e) {
		std::cout << e.what() << std::endl;
	}

	return 0;
}


void textureTest(vr::RenderWindow& window) {

	RenderPass renderPass = createSimpleRenderPass(window);
	
	vr::ShaderPtr vertexShader = window.createShader("TextureVertexShader.spv", VK_SHADER_STAGE_VERTEX_BIT);
	vr::ShaderPtr fragmentShader = window.createShader("TextureFragmentShader.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
	vr::ShaderSet shaderSet = window.createShaderSet(vertexShader, fragmentShader);

	uint32_t pipeline = window.createPipeline(shaderSet, renderPass.renderPass, 0);

	CameraController controller(window);
	controller.sensitivty = 5.f;
	controller.speed = 10.f;

	vr::DescriptorSetPtr descriptorSet = shaderSet.getDescriptorSet(0);
	SceneUbo sceneUbo = {};
	sceneUbo.view = controller.getView(0.0f);
	sceneUbo.proj = controller.getProjection(60.f);

	vr::BufferPtr sceneUniformBuffer = window.createUniformBuffer(sizeof(SceneUbo), &sceneUbo);
	window.updateDescriptorSet(descriptorSet, 0, sceneUniformBuffer, nullptr, nullptr, true);

	ObjectUbo objectUbo = {};
	objectUbo.model = glm::mat4(1.0f);

	vr::BufferPtr objectUniformBuffer = window.createUniformBuffer(sizeof(ObjectUbo), &objectUbo);
	window.updateDescriptorSet(descriptorSet, 1, objectUniformBuffer, nullptr, nullptr, true);

	vr::BufferPtr vertexBuffer = window.createVertexBuffer(sizeof(quad), quad);
	vr::BufferPtr indexBuffer = window.createIndexBuffer(sizeof(quadIndices), quadIndices);


	vr::SamplerPtr sampler = window.createSampler(VK_FILTER_NEAREST);
	int width, height, channels;
	unsigned char* pixels = stbi_load("Box.png", &width, &height, &channels, 0);
	if (!pixels) {
		std::cerr << "Failed to load image" << std::endl;
		return;
	}
	vr::ImagePtr image = window.createTextureImage2D({ (uint32_t)width, (uint32_t)height }, pixels, channels);
	stbi_image_free(pixels);
	window.updateDescriptorSet(descriptorSet, 2, nullptr, image, sampler, true);

	FrameTimer timer;
	while (window.isOpen()) {
		window.pollEvents();

		float dt = timer.getDeltaTime();

		window.beginFrame();
		
		sceneUbo.view = controller.getView(dt);
		memcpy(sceneUniformBuffer->mappedData, &sceneUbo, sizeof(sceneUbo));
		window.updateDescriptorSet(descriptorSet, 0, sceneUniformBuffer, nullptr, nullptr, false);

		window.beginRenderPass(renderPass.renderPass, renderPass.frameBuffer, VK_SUBPASS_CONTENTS_INLINE);
		
		window.bindPipeline(pipeline);

		window.bindVertexBuffer(vertexBuffer);
		window.bindIndexBuffer(indexBuffer);

		window.bindDescriptorSet(descriptorSet, pipeline);

		window.drawIndexed(6, 1);

		window.endRenderPass();

		window.endFrame();
		
	}

}

void computeTest(vr::RenderWindow& window) {
	
	RenderPass renderPass = createSimpleRenderPass(window);
	
	vr::ShaderPtr vshader = window.createShader("ComputeVertexShader.spv", VK_SHADER_STAGE_VERTEX_BIT);
	vr::ShaderPtr fshader = window.createShader("ComputeFragmentShader.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
	vr::ShaderSet shaderSet = window.createShaderSet(vshader, fshader);

	auto pipeline = window.createPipeline(shaderSet, renderPass.renderPass, 0);

	// BOX
	vr::BufferPtr boxVertexbuffer = window.createVertexBuffer(sizeof(box), box);
	vr::BufferPtr boxIndexBuffer = window.createIndexBuffer(sizeof(boxIndices), boxIndices);

	// Compute Shader
	const int count = 2048 * 2;
	vr::ShaderPtr computeShader = window.createShader("ComputeShader.spv", VK_SHADER_STAGE_COMPUTE_BIT);
	vr::ShaderSet computeSet = window.createShaderSet(computeShader);
	auto computePipeline = window.createPipeline(computeSet);
	vr::DescriptorSetPtr computeDescSet = computeSet.getDescriptorSet(0);

	// Compute Shader uniform buffer
	int ccount = count;
	vr::BufferPtr configBuffer = window.createUniformBuffer(sizeof(ccount), &ccount);
	window.updateDescriptorSet(computeDescSet, 0, configBuffer, nullptr, nullptr, true);

	// Compute ouputBuffer
	vr::BufferPtr outputBuffer = window.createStorageBuffer(sizeof(glm::mat4) * count, nullptr);
	window.updateDescriptorSet(computeDescSet, 2, outputBuffer, nullptr, nullptr, true);


	float time = 1.0f;

	// Compute Command buffer
	vr::CommandBufferPtr computeCommandBuffer = window.createCommandBuffer(true);
	window.recordCommandBuffer(computeCommandBuffer, [&](uint32_t frameIndex) {
		window.bindPipeline(computePipeline, computeCommandBuffer, frameIndex);
		window.bindDescriptorSet(computeDescSet, computePipeline, computeCommandBuffer, frameIndex);
		window.pushConstants(computePipeline, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(float), &time, computeCommandBuffer, frameIndex);
		vkCmdDispatch(computeCommandBuffer->buffers[frameIndex], (count / 256) + 1, 1, 1);
		});

	// Dispatch compute shader
	auto fence = window.submitToComputeQueue(computeCommandBuffer);

	// Vertex Shader uniformBuffer
	CameraController controller(window);
	vr::DescriptorSetPtr descriptorSet = shaderSet.getDescriptorSet(0);
	UBO ubo = {};
	ubo.matrixCount = count;
	ubo.view = controller.getView(0.0f);
	ubo.projection = controller.getProjection(60.f);

	vr::BufferPtr uniformBuffer = window.createUniformBuffer(sizeof(ubo), &ubo);

	window.updateDescriptorSet(descriptorSet, 0, uniformBuffer, nullptr, nullptr, true);

	// Wait until compute shader is done
	window.waitForFence(fence);
	// update vertex storage buffer with compute shader output
	window.updateDescriptorSet(descriptorSet, 1, outputBuffer, nullptr, nullptr, true);

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


		window.beginFrame();
		/*
		*/
		// Rerecord compute command buffer with new time
		window.recordCommandBuffer(computeCommandBuffer, [&](uint32_t frameIndex) {
			window.bindPipeline(computePipeline, computeCommandBuffer, frameIndex);
			window.bindDescriptorSet(computeDescSet, computePipeline, computeCommandBuffer, frameIndex);
			window.pushConstants(computePipeline, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(float), &time, computeCommandBuffer, frameIndex);
			vkCmdDispatch(computeCommandBuffer->buffers[frameIndex], (count / 256) + 1, 1, 1);
		}, true);
		// Dispatch compute shader
		fence = window.submitToComputeQueue(computeCommandBuffer);

		// update vertex uniform buffer
		ubo.view = controller.getView(dt);
		memcpy((char*)uniformBuffer->mappedData + offsetof(UBO, UBO::view), &ubo.view, sizeof(glm::mat4));
		window.updateDescriptorSet(descriptorSet, 0, uniformBuffer, nullptr, nullptr, false);
		/*
		*/
		// wait for compute shader to finish
		window.waitForFence(fence);
		// update vertexShader with compute output
		window.updateDescriptorSet(descriptorSet, 1, outputBuffer, nullptr, nullptr, false);

		// draw frame
		window.beginRenderPass(renderPass.renderPass, renderPass.frameBuffer, VK_SUBPASS_CONTENTS_INLINE, glm::vec3(1.0f, 0.0f, 0.0f));

		window.bindPipeline(pipeline);
		
		window.bindVertexBuffer(boxVertexbuffer);
		window.bindIndexBuffer(boxIndexBuffer);

		window.bindDescriptorSet(descriptorSet, pipeline);

		window.drawIndexed(sizeof(boxIndices) / sizeof(uint32_t), count);

		window.endRenderPass();

		window.endFrame();

	}
}

RenderPass createSimpleRenderPass(vr::RenderWindow& window) {
	RenderPass renderPassStruct = {};

	renderPassStruct.depthImage = window.createDepthImage(window.getCurrentExtent());

	std::vector<VkAttachmentDescription> attachments(2);
	attachments[0] = window.getSwapchainAttachment();
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

	renderPassStruct.renderPass = window.createRenderPass(attachments, subpasses, { });

	renderPassStruct.frameBuffer = window.createFramebuffer(renderPassStruct.renderPass, { renderPassStruct.depthImage });

	return std::move(renderPassStruct);
}
