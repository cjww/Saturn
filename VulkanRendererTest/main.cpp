#define DEBUG_LOG

#include <iostream>


#include <Renderer.hpp>
#include <RenderWindow.hpp>
#include <Tools\Logger.hpp>

#include <chrono>
#include <array>

#include "glm\common.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include "glm/gtx/transform.hpp"


class CameraController {
private:
	sa::RenderWindow& window;
	glm::vec3 viewForward;
	glm::vec3 viewPos;
	glm::vec2 lastMousePos;

	glm::mat4 mat = glm::mat4(1);
	bool escapePressed;

public:
	float speed = 10.f;
	float sensitivty = 1.f;
		
	bool mouseLocked;

	CameraController(sa::RenderWindow& window) : window(window) {
		viewForward = glm::vec3(0, 0, 1);
		viewPos = glm::vec3(0.f);
		
		mouseLocked = true;
		window.setCursorDisabled(true);
	};

	glm::mat4 getView(float dt) {
		float hori = window.getKey(sa::Key::D) - window.getKey(sa::Key::A);
		float vert = window.getKey(sa::Key::W) - window.getKey(sa::Key::S);

	  	glm::vec2 mPos = { window.getCursorPosition().x, window.getCursorPosition().y };
		glm::vec2 center = glm::vec2((float)window.getCurrentExtent().width / 2, (float)window.getCurrentExtent().height / 2);
		glm::vec2 diff = mPos - center;
		
		float up = window.getKey(sa::Key::SPACE) - window.getKey(sa::Key::LEFT_CONTROL);
		bool sprint = window.getKey(sa::Key::LEFT_SHIFT);
		

		if (sa::Window::IsGamepad(sa::Joystick::JOYSTICK_1)) {
			sa::GamepadState state = sa::Window::GetGamepadState(sa::Joystick::JOYSTICK_1);

			hori = state.getAxis(sa::GamepadAxis::LEFT_X);
			vert = state.getAxis(sa::GamepadAxis::LEFT_Y) * -1.f;

			diff.x = state.getAxis(sa::GamepadAxis::RIGHT_X);
			diff.y = state.getAxis(sa::GamepadAxis::RIGHT_Y);

			up = state.getAxis(sa::GamepadAxis::LEFT_TRIGGER) - state.getAxis(sa::GamepadAxis::RIGHT_TRIGGER);

			sprint = state.getButton(sa::GamepadButton::RIGHT_BUMPER);

		}

		if (mouseLocked) {
			window.setCursorPosition({ center.x, center.y });
		}
		else {
			diff = glm::vec2(0, 0);
		}

		if (window.getKey(sa::Key::ESCAPE) && !escapePressed) {
			mouseLocked = !mouseLocked;
			window.setCursorDisabled(mouseLocked);
		}
		escapePressed = window.getKey(sa::Key::ESCAPE);


		viewForward = glm::vec3(glm::vec4(viewForward, 0.0f) * glm::rotate(diff.x * dt * sensitivty, glm::vec3(0, 1, 0)));

		glm::vec3 right = glm::cross(viewForward, glm::vec3(0, 1, 0));
		viewForward = glm::vec3(glm::vec4(viewForward, 0.0f) * glm::rotate(diff.y * dt * sensitivty, right));


		float finalSpeed = speed;
		if (sprint) {
			finalSpeed *= 2;
		}

		viewPos += right * hori * dt * finalSpeed;
		viewPos += viewForward * vert * dt * finalSpeed;
		viewPos += glm::vec3(0, 1, 0) * up * dt * finalSpeed;

		
		return glm::lookAt(viewPos, viewPos + viewForward, glm::vec3(0, 1, 0));
	}
	/*
	glm::mat4 getViewRayTracing(float dt) {
		int hori = window.getKey(sa::Key::D) - window.getKey(sa::Key::A);
		int vert = window.getKey(sa::Key::W) - window.getKey(sa::Key::S);

		glm::vec2 mPos = { window.getCursorPosition().x, window.getCursorPosition().y };
		glm::vec2 center = glm::vec2((float)window.getCurrentExtent().width / 2, (float)window.getCurrentExtent().height / 2);
		glm::vec2 diff = mPos - center;
		if (mouseLocked) {
			window.setCursorPosition({ center.x, center.y });
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
	*/

	glm::mat4 getProjection(float fovDegrees) {
		auto projection = glm::perspective(glm::radians(fovDegrees), (float)window.getCurrentExtent().width / window.getCurrentExtent().height, 0.01f, 1000.0f);
		projection[1][1] *= -1;
		return projection;
	}
};
/*

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
*/

struct UBO {
	glm::mat4 view;
	glm::mat4 projection;
};

struct PushConstant {
	glm::mat4 world;
};


struct VertexColorUV {
	float position[4];
	float color[4];
	float uv[2];
};


std::array<VertexColorUV, 4> quad = {
	VertexColorUV{ { -0.5f, -0.5f, 0.0f, 1.0f },{ 1.0f, 0.0f, 0.0f, 1.0f }, {0.0f, 1.0f} },
	VertexColorUV{ { 0.5f, -0.5f, 0.0f, 1.0f },	{ 0.0f, 1.0f, 0.0f, 1.0f }, {1.0f, 1.0f} },
	VertexColorUV{ { 0.5f, 0.5f, 0.0f, 1.0f },	{ 0.0f, 0.0f, 1.0f, 1.0f }, {1.0f, 0.0f} },
	VertexColorUV{ { -0.5f, 0.5f, 0.0f, 1.0f },	{ 1.0f, 1.0f, 1.0f, 1.0f }, {0.0f, 0.0f} }
};


std::array<VertexColorUV, 8> box = {
	// forward
	VertexColorUV{ { -1.0f, 1.0f, 1.0f, 1.0f }, { 1, 1, 1, 1 }, { 0, 1 } },
	VertexColorUV{ { 1.0f, 1.0f, 1.0f, 1.0f }, { 1, 1, 1, 1 }, { 1, 1 } },
	VertexColorUV{ { -1.0f, -1.0f, 1.0f, 1.0f }, { 1, 1, 1, 1 }, { 1, 0 } },
	VertexColorUV{ { 1.0f, -1.0f, 1.0f, 1.0f }, { 1, 1, 1, 1 }, { 0, 0 } },
	// back
	VertexColorUV{ { -1.0f, 1.0f, -1.0f, 1.0f }, { 1, 1, 1, 1 }, { 0, 1 } },
	VertexColorUV{ { 1.0f, 1.0f, -1.0f, 1.0f }, { 1, 1, 1, 1 }, { 1, 1 } },
	VertexColorUV{ { -1.0f, -1.0f, -1.0f, 1.0f }, { 1, 1, 1, 1 }, { 1, 0 } },
	VertexColorUV{ { 1.0f, -1.0f, -1.0f, 1.0f }, { 1, 1, 1, 1 }, { 0, 0 } }
};

std::array<uint32_t, 36> boxIndices = {
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

void deffered(sa::RenderWindow& window) {
	sa::Renderer& renderer = sa::Renderer::get();

	// FIRST PASS
	sa::Texture2D colorTexture = renderer.createTexture2D(
		sa::TextureTypeFlagBits::COLOR_ATTACHMENT |
		sa::TextureTypeFlagBits::INPUT_ATTACHMENT,
		window.getCurrentExtent());

	sa::Texture2D depthTexture = renderer.createTexture2D(
		sa::TextureTypeFlagBits::DEPTH_ATTACHMENT,
		window.getCurrentExtent());

	sa::Texture2D positionsTexture = renderer.createTexture2D(
		sa::TextureTypeFlagBits::COLOR_ATTACHMENT |
		sa::TextureTypeFlagBits::INPUT_ATTACHMENT,
		window.getCurrentExtent(),
		sa::FormatPrecisionFlagBits::e32Bit,
		sa::FormatDimensionFlagBits::e4,
		sa::FormatTypeFlagBits::SFLOAT);

	sa::Texture2D shadedTexture = renderer.createTexture2D(
		sa::TextureTypeFlagBits::COLOR_ATTACHMENT |
		sa::TextureTypeFlagBits::SAMPLED,
		window.getCurrentExtent());

	sa::Texture2D brightnessTexture = renderer.createTexture2D(
		sa::TextureTypeFlagBits::COLOR_ATTACHMENT |
		sa::TextureTypeFlagBits::SAMPLED,
		window.getCurrentExtent());


	auto defferedProgram = renderer.createRenderProgram()
		.addColorAttachment(false, colorTexture) // 0 color
		.addDepthAttachment() // 1 depth
		.addColorAttachment(false, positionsTexture) // 2 positions
		.addColorAttachment(true, shadedTexture) // 3 combine
		.addColorAttachment(true, brightnessTexture) // 4 brightness
		.beginSubpass()
		.addAttachmentReference(0, sa::SubpassAttachmentUsage::ColorTarget)
		.addAttachmentReference(1, sa::SubpassAttachmentUsage::DepthTarget)
		.addAttachmentReference(2, sa::SubpassAttachmentUsage::ColorTarget)
		.endSubpass()
		.beginSubpass()
		.addAttachmentReference(3, sa::SubpassAttachmentUsage::ColorTarget)
		.addAttachmentReference(4, sa::SubpassAttachmentUsage::ColorTarget)
		.addAttachmentReference(0, sa::SubpassAttachmentUsage::Input)
		.addAttachmentReference(2, sa::SubpassAttachmentUsage::Input)
		.endSubpass()
		.end();


	auto gFramebuffer = renderer.createFramebuffer(
		defferedProgram,
		{ colorTexture, depthTexture, positionsTexture, shadedTexture, brightnessTexture });

	auto defferedPipeline = renderer.createGraphicsPipeline(
		defferedProgram,
		0,
		window.getCurrentExtent(),
		"TestShader.vert.spv",
		"TestShader.frag.spv");

	auto combinePipeline = renderer.createGraphicsPipeline(
		defferedProgram,
		1,
		window.getCurrentExtent(),
		"PostProcess.vert.spv",
		"PostProcess.frag.spv");

	ResourceID defferedDescriptorSet = renderer.allocateDescriptorSet(defferedPipeline, 0);


	sa::Buffer sceneUniformBuffer = renderer.createBuffer(
		sa::BufferType::UNIFORM);

	CameraController camera(window);

	UBO ubo = {};
	ubo.view = glm::lookAt(glm::vec3{ 0, 0, 1 }, { 0, 0, 0 }, { 0, 1, 0 });
	ubo.projection = camera.getProjection(90);

	sceneUniformBuffer.write(ubo);
	renderer.updateDescriptorSet(defferedDescriptorSet, 0, sceneUniformBuffer);


	sa::Image boxImage("Box.png");
	sa::Texture2D texture = renderer.createTexture2D(boxImage);
	ResourceID sampler = renderer.createSampler(sa::FilterMode::LINEAR);
	renderer.updateDescriptorSet(defferedDescriptorSet, 2, texture, sampler);


	ResourceID combineDescriptorSet = renderer.allocateDescriptorSet(combinePipeline, 0);
	renderer.updateDescriptorSet(combineDescriptorSet, 0, colorTexture);
	renderer.updateDescriptorSet(combineDescriptorSet, 1, positionsTexture);

	// SECOND PASS

	auto mainRenderProgram = renderer.createRenderProgram()
		.addSwapchainAttachment(window.getSwapchainID()) // 0 swapchain
		.beginSubpass()
		.addAttachmentReference(0, sa::SubpassAttachmentUsage::ColorTarget)
		.endSubpass()
		.end();

	auto mainFramebuffer = renderer.createSwapchainFramebuffer(
		mainRenderProgram,
		window.getSwapchainID(),
		{ });

	auto mainPipeline = renderer.createGraphicsPipeline(
		mainRenderProgram,
		0,
		window.getCurrentExtent(),
		"PostProcess.vert.spv",
		"MainShader.frag.spv");

	auto mainDescriptorSet = renderer.allocateDescriptorSet(mainPipeline, 0);
	renderer.updateDescriptorSet(mainDescriptorSet, 0, shadedTexture, sampler);

	sa::Buffer vertexBuffer = renderer.createBuffer(
		sa::BufferType::VERTEX, quad.size() * sizeof(VertexColorUV), quad.data());
	sa::Buffer indexBuffer = renderer.createBuffer(
		sa::BufferType::INDEX);
	indexBuffer.write<uint32_t, 6>({
		0, 1, 3,
		1, 2, 3
		});

	sa::Buffer boxVertexBuffer = renderer.createBuffer(
		sa::BufferType::VERTEX);
	boxVertexBuffer.write(box);
	sa::Buffer boxIndexBuffer = renderer.createBuffer(
		sa::BufferType::INDEX);
	boxIndexBuffer.write(boxIndices);




	//Compute
	ResourceID computePipeline = renderer.createComputePipeline("ComputeShader.comp.spv");
	ResourceID computeDescriptorSet = renderer.allocateDescriptorSet(computePipeline, 0);

	std::vector<glm::mat4> data(100000, glm::mat4(1));

	sa::Buffer configBuffer = renderer.createBuffer(sa::BufferType::UNIFORM);
	configBuffer.write(data.size());

	sa::Buffer outputBuffer = renderer.createBuffer(sa::BufferType::STORAGE);
	outputBuffer.write(data);

	renderer.updateDescriptorSet(computeDescriptorSet, 0, configBuffer);
	renderer.updateDescriptorSet(computeDescriptorSet, 2, outputBuffer);

	// Compute Blur
	ResourceID blurPipeline = renderer.createComputePipeline("BlurShader.comp.spv");
	ResourceID blurDescriptorSet = renderer.allocateDescriptorSet(blurPipeline, 0);

	sa::Extent extent = {
		(uint32_t)(window.getCurrentExtent().width / 4.f),
		(uint32_t)(window.getCurrentExtent().height / 4.f)
	};
	//sa::Extent extent = window.getCurrentExtent();
	sa::Texture2D blurredTexture = renderer.createTexture2D(
		sa::TextureTypeFlagBits::STORAGE | sa::TextureTypeFlagBits::SAMPLED,
		extent,
		sa::FormatPrecisionFlagBits::e32Bit, sa::FormatDimensionFlagBits::e4, sa::FormatTypeFlagBits::SFLOAT);

	sa::Buffer blurConfigBuffer = renderer.createBuffer(
		sa::BufferType::UNIFORM,
		sizeof(extent), &extent);



	renderer.updateDescriptorSet(blurDescriptorSet, 0, blurConfigBuffer);
	renderer.updateDescriptorSet(blurDescriptorSet, 1, brightnessTexture, sampler);
	renderer.updateDescriptorSet(blurDescriptorSet, 2, blurredTexture);

	renderer.updateDescriptorSet(mainDescriptorSet, 1, blurredTexture, sampler);

	sa::DirectContext tmp = renderer.createDirectContext();
	tmp.begin(sa::ContextUsageFlagBits::SIMULTANEOUS_USE);
	tmp.transitionTexture(blurredTexture, sa::Transition::NONE, sa::Transition::COMPUTE_SHADER_WRITE);
	tmp.end();
	tmp.submit();
	tmp.destroy();

	sa::SubContext subContext = renderer.createSubContext(gFramebuffer, defferedProgram, 1);
	/*
	subContext.preRecord([=](sa::RenderContext& context) {
		context.bindPipeline(combinePipeline);
		context.bindVertexBuffers(0, { vertexBuffer });
		context.bindIndexBuffer(indexBuffer);
		context.bindDescriptorSet(combineDescriptorSet, combinePipeline);
		context.drawIndexed(6, 1);
	}, sa::ContextUsageFlagBits::RENDER_PROGRAM_CONTINUE);
	*/


	
	auto now = std::chrono::high_resolution_clock::now();
	float dt = 0;
	float timer = 0.0f;

	while (window.isOpen()) {
		window.pollEvents();

		timer += dt;
		ubo.view = camera.getView(dt);

		window.setWindowTitle("FPS: " + std::to_string(1 / dt));

		data = outputBuffer.getContent<glm::mat4>();


		sa::RenderContext context = window.beginFrame();
		if (context) {

			
			subContext.begin(sa::ContextUsageFlagBits::RENDER_PROGRAM_CONTINUE | sa::ContextUsageFlagBits::ONE_TIME_SUBMIT);
			subContext.bindPipeline(combinePipeline);
			subContext.bindVertexBuffers(0, { vertexBuffer });
			subContext.bindIndexBuffer(indexBuffer);
			subContext.bindDescriptorSet(combineDescriptorSet, combinePipeline);
			subContext.drawIndexed(6, 1);
			subContext.end();

			sceneUniformBuffer.write(ubo);
			context.updateDescriptorSet(defferedDescriptorSet, 0, sceneUniformBuffer);
			context.updateDescriptorSet(defferedDescriptorSet, 1, outputBuffer);


			context.transitionTexture(blurredTexture, sa::Transition::RENDER_PROGRAM_INPUT, sa::Transition::COMPUTE_SHADER_WRITE);


			context.bindPipeline(computePipeline);
			context.bindDescriptorSet(computeDescriptorSet, computePipeline);
			context.pushConstant(computePipeline, sa::ShaderStageFlagBits::COMPUTE, 0, timer);

			int groupcount = ((data.size()) / 256) + 1;
			context.dispatch(groupcount, 1, 1);

			context.beginRenderProgram(defferedProgram, gFramebuffer);

			context.bindPipeline(defferedPipeline);
			context.bindDescriptorSet(defferedDescriptorSet, defferedPipeline);
			// Drawing
			context.bindVertexBuffers(0, { boxVertexBuffer });
			context.bindIndexBuffer(boxIndexBuffer);

			context.drawIndexed(boxIndices.size(), data.size());

			context.nextSubpass(sa::SubpassContents::SUB_CONTEXT);
			/*
			context.bindPipeline(combinePipeline);
			context.bindVertexBuffers(0, { vertexBuffer });
			context.bindIndexBuffer(indexBuffer);
			context.bindDescriptorSet(combineDescriptorSet, combinePipeline);

			context.drawIndexed(6, 1);
			*/
			context.executeSubContext(subContext);

			context.endRenderProgram(defferedProgram);


			context.bindPipeline(blurPipeline);
			context.bindDescriptorSet(blurDescriptorSet, blurPipeline);
			int groupcountX = (extent.width / 32) + 1;
			int groupcountY = (extent.height / 32) + 1;
			context.dispatch(groupcountX, groupcountY, 1);

			//context.barrier(blurredTexture);

			context.transitionTexture(blurredTexture, sa::Transition::COMPUTE_SHADER_WRITE, sa::Transition::RENDER_PROGRAM_INPUT);

			context.beginRenderProgram(mainRenderProgram, mainFramebuffer);
			context.bindPipeline(mainPipeline);
			context.bindDescriptorSet(mainDescriptorSet, mainPipeline);
			context.drawIndexed(6, 1);

			context.endRenderProgram(mainRenderProgram);

			window.display();
		}

		dt = std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - now).count();
		now = std::chrono::high_resolution_clock::now();

	}
}

void forward(sa::RenderWindow& window) {
	sa::Renderer& renderer = sa::Renderer::get();

	window.setResizeCallback([](sa::Extent newExtent) {
		DEBUG_LOG_INFO("CALLBACK: new Extent: ", newExtent.width, newExtent.height);
	});

	// FIRST PASS
	sa::Texture2D colorTexture = renderer.createTexture2D(
		sa::TextureTypeFlagBits::COLOR_ATTACHMENT,
		window.getCurrentExtent(), window.getSwapchainID(), 8);

	sa::Texture2D depthTexture = renderer.createTexture2D(
		sa::TextureTypeFlagBits::DEPTH_ATTACHMENT,
		window.getCurrentExtent(), 8);


	auto mainRenderProgram = renderer.createRenderProgram()
		.addSwapchainAttachment(window.getSwapchainID()) // 0 swapchain
		.addColorAttachment(false, colorTexture) // 1 color
		.addDepthAttachment(depthTexture) // 2 depth
		.beginSubpass()
			.addAttachmentReference(1, sa::SubpassAttachmentUsage::ColorTarget)
			.addAttachmentReference(2, sa::SubpassAttachmentUsage::DepthTarget)
			.addAttachmentReference(0, sa::SubpassAttachmentUsage::Resolve)
		.endSubpass()
		.end();


	renderer.setClearColor(mainRenderProgram, { 0.2, 0.3, 0.3, 1 });

	auto mainFramebuffer = renderer.createSwapchainFramebuffer(
		window.getSwapchainID(),
		mainRenderProgram,
		{ colorTexture, depthTexture });

	auto mainPipeline = renderer.createGraphicsPipeline(
		mainRenderProgram,
		0,
		window.getCurrentExtent(),
		"ForwardShader.vert.spv",
		"ForwardShader.frag.spv");


	ResourceID mainDescriptorSet = renderer.allocateDescriptorSet(mainPipeline, 0);

	sa::Buffer uniformBuffer = renderer.createBuffer(
		sa::BufferType::UNIFORM);

	CameraController camera(window);

	UBO ubo = {};
	ubo.view = glm::lookAt(glm::vec3{ 0, 0, 1 }, { 0, 0, 0 }, { 0, 1, 0 });
	ubo.projection = camera.getProjection(90);

	uniformBuffer.write(ubo);
	renderer.updateDescriptorSet(mainDescriptorSet, 0, uniformBuffer);


	sa::Image image("Box.png");
	sa::Texture2D texture = renderer.createTexture2D(image);
	ResourceID sampler = renderer.createSampler(sa::FilterMode::LINEAR);
	renderer.updateDescriptorSet(mainDescriptorSet, 1, texture, sampler);


	sa::Buffer vertexBuffer = renderer.createBuffer(
		sa::BufferType::VERTEX, quad.size() * sizeof(VertexColorUV), quad.data());
	sa::Buffer indexBuffer = renderer.createBuffer(
		sa::BufferType::INDEX);
	indexBuffer.write<uint32_t, 6>({
		0, 1, 3,
		1, 2, 3
		});

	sa::Buffer boxVertexBuffer = renderer.createBuffer(
		sa::BufferType::VERTEX);
	boxVertexBuffer.write(box);
	sa::Buffer boxIndexBuffer = renderer.createBuffer(
		sa::BufferType::INDEX);
	boxIndexBuffer.write(boxIndices);

	auto now = std::chrono::high_resolution_clock::now();
	float dt = 0;
	float timer = 0.0f;

	while (window.isOpen()) {
		window.pollEvents();

		timer += dt;
		ubo.view = camera.getView(dt);

		window.setWindowTitle("FPS: " + std::to_string(1 / dt));


		sa::RenderContext context = window.beginFrame();
		if (context) {

			uniformBuffer.write(ubo);
			context.updateDescriptorSet(mainDescriptorSet, 0, uniformBuffer);
			

			context.beginRenderProgram(mainRenderProgram, mainFramebuffer);

			context.bindPipeline(mainPipeline);
			context.bindDescriptorSet(mainDescriptorSet, mainPipeline);
			glm::mat4 world(1);
			context.pushConstant(mainPipeline, sa::ShaderStageFlagBits::VERTEX, 0, world);
			
			// Drawing
			context.bindVertexBuffers(0, { boxVertexBuffer });
			context.bindIndexBuffer(boxIndexBuffer);

			context.drawIndexed(boxIndices.size(), 1);

			context.endRenderProgram(mainRenderProgram);

			window.display();
		}

		dt = std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - now).count();
		now = std::chrono::high_resolution_clock::now();

	}
}

int main() {

#ifdef _WIN32
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif


	try {
		const int WIDTH = 1400, HEIGHT = 800;
		sa::RenderWindow window(WIDTH, HEIGHT, "Test Window");
		
		ResourceID crossHairCursor = sa::Window::CreateCursor(sa::StandardCursor::CROSSHAIR);
		window.setCursor(crossHairCursor);

		sa::Window::SetJoystickConnectedCallback([](sa::Joystick joystick, sa::ConnectionState state) {
			if (state == sa::ConnectionState::CONNECTED) {
				DEBUG_LOG_INFO("Joystick", (int)joystick, "connected: ", sa::Window::GetJoystickName(joystick));
			}
			else {
				DEBUG_LOG_INFO("Joystick", (int)joystick, "disconnected");
			}
		});

		
		deffered(window);
		//forward(window);


	}
	catch (const std::exception& e) {
		DEBUG_LOG_ERROR(e.what());
		
	}

	return 0;
}
