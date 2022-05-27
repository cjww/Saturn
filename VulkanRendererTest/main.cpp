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
	float sensitivty = 10.f;
		
	bool mouseLocked;

	CameraController(sa::RenderWindow& window) : window(window) {
		viewForward = glm::vec3(0, 0, 1);
		viewPos = glm::vec3(0.f);
		
		mouseLocked = true;
		window.setHideCursor(true);
	};

	glm::mat4 getView(float dt) {
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

		if (window.getKey(sa::Key::ESCAPE) && !escapePressed) {
			mouseLocked = !mouseLocked;
			window.setHideCursor(mouseLocked);
		}
		escapePressed = window.getKey(sa::Key::ESCAPE);

		int up = window.getKey(sa::Key::SPACE) - window.getKey(sa::Key::LEFT_CONTROL);
		bool sprint = window.getKey(sa::Key::LEFT_SHIFT);

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


int main() {

#ifdef _WIN32
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

		const int WIDTH = 1000, HEIGHT = 600;
		sa::RenderWindow window(WIDTH, HEIGHT, "Test Window");
		sa::Renderer& renderer = sa::Renderer::get();
		
		sa::Texture2D depthTexture = renderer.createTexture2D(
			sa::TextureTypeFlagBits::DEPTH_ATTACHMENT, 
			window.getCurrentExtent());
		sa::Texture2D colorTexture = renderer.createTexture2D(
			sa::TextureTypeFlagBits::COLOR_ATTACHMENT | 
			sa::TextureTypeFlagBits::INPUT_ATTACHMENT,
			window.getCurrentExtent());

		sa::Texture2D positionsTexture = renderer.createTexture2D(
			sa::TextureTypeFlagBits::COLOR_ATTACHMENT |
			sa::TextureTypeFlagBits::INPUT_ATTACHMENT,
			window.getCurrentExtent(),
			sa::FormatPrecisionFlagBits::e32Bit, 
			sa::FormatDimensionFlagBits::e4, 
			sa::FormatTypeFlagBits::SFLOAT);



		auto mainRenderProgram = renderer.createRenderProgram()
			.addSwapchainAttachment(window.getSwapchainID()) // 0 swapchain
			.addDepthAttachment() // 1 depth
			.addColorAttachment(false) // 2 color
			.addColorAttachment(false, positionsTexture) // 3 positions
			.beginSubpass()
				.addAttachmentReference(2, sa::SubpassAttachmentUsage::ColorTarget)
				.addAttachmentReference(3, sa::SubpassAttachmentUsage::ColorTarget)
				.addAttachmentReference(1, sa::SubpassAttachmentUsage::DepthTarget)
			.endSubpass()
			.beginSubpass()
				.addAttachmentReference(0, sa::SubpassAttachmentUsage::ColorTarget)
				.addAttachmentReference(2, sa::SubpassAttachmentUsage::Input)
				.addAttachmentReference(3, sa::SubpassAttachmentUsage::Input)
			.endSubpass()
			.end();
		

		sa::Color clearColor = { 0.2f, 0.7f, 0.8f, 1.f };
		renderer.setClearColor(mainRenderProgram, clearColor);

		auto mainFramebuffer = renderer.createSwapchainFramebuffer(
			mainRenderProgram, 
			window.getSwapchainID(), 
			{ depthTexture, colorTexture, positionsTexture });
		
		auto mainPipeline = renderer.createGraphicsPipeline(
			mainRenderProgram,
			0,
			window.getCurrentExtent(),
			"TestShader.vert.spv",
			"TestShader.frag.spv");


		auto postPipeline = renderer.createGraphicsPipeline(
			mainRenderProgram,
			1,
			window.getCurrentExtent(),
			"PostProcess.vert.spv",
			"PostProcess.frag.spv");



		sa::Buffer vertexBuffer = renderer.createBuffer(
			sa::BufferType::VERTEX, quad.size() * sizeof(VertexColorUV), quad.data());
		sa::Buffer indexBuffer = renderer.createBuffer(
			sa::BufferType::INDEX);

		indexBuffer.write<uint32_t, 6>({
			0, 1, 3,
			1, 2, 3
		});


		ResourceID descriptorSet = renderer.allocateDescriptorSet(mainPipeline, 0, window.getSwapchainImageCount());

		sa::Buffer uniformBuffer = renderer.createBuffer(
			sa::BufferType::UNIFORM);

		CameraController camera(window);

		UBO ubo = {};
		ubo.view = glm::lookAt(glm::vec3{ 0, 0, 1 }, { 0, 0, 0 }, { 0, 1, 0 });
		ubo.projection = camera.getProjection(90);

		uniformBuffer.write(ubo); 
		renderer.updateDescriptorSet(descriptorSet, 0, uniformBuffer);




		std::vector<PushConstant> objects;

		PushConstant pc = {};
		pc.world = glm::mat4(1);
		objects.push_back(pc);

		pc.world = glm::translate(pc.world, glm::vec3(0.5, 0, 0));
		objects.push_back(pc);


		ResourceID descriptorSetPost = renderer.allocateDescriptorSet(postPipeline, 0, window.getSwapchainImageCount());

		sa::Image image("Box.png");
		sa::Texture2D texture = renderer.createTexture2D(image);
		ResourceID sampler = renderer.createSampler();
		renderer.updateDescriptorSet(descriptorSet, 1, texture, sampler);

		renderer.updateDescriptorSet(descriptorSetPost, 0, colorTexture);
		renderer.updateDescriptorSet(descriptorSetPost, 1, positionsTexture);

		auto now = std::chrono::high_resolution_clock::now();
		float dt = 0;
		float timer = 0.0f;
		while (window.isOpen()) {
			window.pollEvents();
			
			timer += dt;
			ubo.view = camera.getView(dt);

			objects[0].world = glm::rotate(objects[0].world, dt, {0.f, 1.0f, 0.f});
			objects[1].world = glm::translate(glm::mat4(1), { 0.f, std::sin(timer * 2), 0.f });

			window.setWindowTitle("FPS: " + std::to_string(1 / dt));

			sa::RenderContext context = window.beginFrame();
			if (context) {
				
				uniformBuffer.write(ubo);
				context.updateDescriptorSet(descriptorSet, 0, uniformBuffer);

				context.beginRenderProgram(mainRenderProgram, mainFramebuffer);
					context.bindPipeline(mainPipeline);
					context.bindDescriptorSet(descriptorSet, mainPipeline);
					// Drawing
					context.bindVertexBuffers(0, { vertexBuffer });
					context.bindIndexBuffer(indexBuffer);
					for (const auto& object : objects) {
						context.pushConstant(mainPipeline, sa::ShaderStageFlagBits::VERTEX, 0, object);

						context.drawIndexed(indexBuffer.getElementCount<uint32_t>(), 1);
					}

				context.nextSubpass();

					context.bindPipeline(postPipeline);
					context.bindDescriptorSet(descriptorSetPost, postPipeline);
					
					context.drawIndexed(6, 1);
				context.endRenderProgram(mainRenderProgram);
				
				window.display();
			}

			dt = std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - now).count();
			
			now = std::chrono::high_resolution_clock::now();

		}

		texture.destroy();
		depthTexture.destroy();
		colorTexture.destroy();
		positionsTexture.destroy();
	try {
	}
	catch (const std::exception& e) {
		DEBUG_LOG_ERROR(e.what());
		
	}

	return 0;
}
