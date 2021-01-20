#include <iostream>

#include <RenderWindow.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include <chrono>

class CameraController {
private:
	vr::RenderWindow& window;
	glm::vec2 mouseLastPos;
	glm::vec3 viewForward;
	glm::vec3 viewPos;
public:
	float speed = 100.f;

	CameraController(vr::RenderWindow& window) : window(window) {
		mouseLastPos = glm::vec2(0.f);
		viewForward = glm::vec3(0, 0, 1);
		viewPos = glm::vec3(0.f);
	};

	glm::mat4 getView(float dt) {
		int hori = glfwGetKey(window.getWindowHandle(), GLFW_KEY_D) - glfwGetKey(window.getWindowHandle(), GLFW_KEY_A);
		int vert = glfwGetKey(window.getWindowHandle(), GLFW_KEY_W) - glfwGetKey(window.getWindowHandle(), GLFW_KEY_S);

		glm::vec2 mPos = window.getCursorPosition();
		glm::vec2 diff = mouseLastPos - mPos;
		mouseLastPos = mPos;

		viewForward = glm::vec3(glm::vec4(viewForward, 0.0f) * glm::rotate(-diff.x * dt, glm::vec3(0, 1, 0)));

		glm::vec3 right = glm::cross(viewForward, glm::vec3(0, 1, 0));
		viewForward = glm::vec3(glm::vec4(viewForward, 0.0f) * glm::rotate(-diff.y * dt, right));


		viewPos += right * (float)hori * dt * speed;
		viewPos += viewForward * (float)vert * dt * speed;


		return glm::lookAt(viewPos, viewPos + viewForward, glm::vec3(0, 1, 0));
	}

	glm::mat4 getProjection(float fovDegrees) {
		auto projection = glm::perspective(glm::radians(fovDegrees), (float)window.getCurrentExtent().width / window.getCurrentExtent().height, 0.01f, 1000.0f);
		projection[1][1] *= -1;
		return projection;
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
	window.updateDescriptorSet(computeDescSet, 0, configBuffer, nullptr, true);

	// Compute ouputBuffer
	vr::BufferPtr outputBuffer = window.createStorageBuffer(sizeof(glm::mat4) * count, nullptr);
	window.updateDescriptorSet(computeDescSet, 2, outputBuffer, nullptr, true);


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

	window.updateDescriptorSet(descriptorSet, 0, uniformBuffer, nullptr, true);

	// Wait until compute shader is done
	window.waitForFence(fence);
	// update vertex storage buffer with compute shader output
	window.updateDescriptorSet(descriptorSet, 1, outputBuffer, nullptr, true);

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
		window.updateDescriptorSet(descriptorSet, 0, uniformBuffer, nullptr, false);
		/*
		*/
		// wait for compute shader to finish
		window.waitForFence(fence);
		// update vertexShader with compute output
		window.updateDescriptorSet(descriptorSet, 1, outputBuffer, nullptr, false);

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
