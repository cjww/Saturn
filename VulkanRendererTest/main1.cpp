




#include <iostream>

#define IM_VEC2_CLASS_EXTRA											\
        constexpr ImVec2(const glm::vec2& f) : x(f.x), y(f.y) {}	\
        operator glm::vec2() const { return glm::vec2(x ,y); }

#define IM_VEC4_CLASS_EXTRA															\
        constexpr ImVec4(const glm::vec4& f) : x(f.x), y(f.y), z(f.z), w(f.w) {}	\
        operator glm::vec4() const { return glm::vec4(x, y, z, w); }

#include "glm\common.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include "glm/gtx/transform.hpp"

#define SA_DEBUG_LOG
#include <Renderer.hpp>
#include <RenderWindow.hpp>
#define SA_DEBUG_LOG_ENABLE 1
#include <Tools\Logger.hpp>

#include <chrono>
#include <array>
#include <thread>



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
	
	glm::mat4 getProjection(float fovDegrees) {
		auto projection = glm::perspective(glm::radians(fovDegrees), (float)window.getCurrentExtent().width / window.getCurrentExtent().height, 0.01f, 1000.0f);
		projection[1][1] *= -1;
		return projection;
	}
};


struct UBO {
	glm::mat4 view;
	glm::mat4 projection;
};

struct PushConstant {
	glm::mat4 world;
};

struct Vertex{
	float position[4];
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

std::array<uint32_t, 6> quadIndices = {
	0, 1, 3,
	1, 2, 3
};

std::array<VertexColorUV, 8> box = {
	// forward
	VertexColorUV{ { -1.0f, 1.0f, 1.0f, 1.0f }, { 1, 1, 1, 1 }, { 0, 1 } },
	VertexColorUV{ { 1.0f, 1.0f, 1.0f, 1.0f }, { 1, 1, 1, 1 }, { 1, 1 } },
	VertexColorUV{ { -1.0f, -1.0f, 1.0f, 1.0f }, { 1, 1, 1, 1 }, { 0, 0 } },
	VertexColorUV{ { 1.0f, -1.0f, 1.0f, 1.0f }, { 1, 1, 1, 1 }, { 1, 0 } },
	// back
	VertexColorUV{ { -1.0f, 1.0f, -1.0f, 1.0f }, { 1, 1, 1, 1 }, { 0, 1 } },
	VertexColorUV{ { 1.0f, 1.0f, -1.0f, 1.0f }, { 1, 1, 1, 1 }, { 1, 1 } },
	VertexColorUV{ { -1.0f, -1.0f, -1.0f, 1.0f }, { 1, 1, 1, 1 }, { 0, 0 } },
	VertexColorUV{ { 1.0f, -1.0f, -1.0f, 1.0f }, { 1, 1, 1, 1 }, { 1, 0 } }
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
				SA_DEBUG_LOG_INFO("Joystick", (int)joystick, "connected: ", sa::Window::GetJoystickName(joystick));
			}
			else {
				SA_DEBUG_LOG_INFO("Joystick", (int)joystick, "disconnected");
			}
		});


		sa::Renderer& renderer = sa::Renderer::get();

		ResourceID renderProgram = renderer.createRenderProgram()
			.addSwapchainAttachment(window.getSwapchainID())
			.beginSubpass()
			.addAttachmentReference(0, sa::SubpassAttachmentUsage::ColorTarget)
			.endSubpass()
			.end();

		std::vector<sa::DynamicTexture> attachments;
		ResourceID framebuffer = renderer.createSwapchainFramebuffer(renderProgram, window.getSwapchainID(), attachments);

		auto vshaderCode = sa::CompileGLSLFromFile("Passthrough.vert", sa::VERTEX);

		//auto vshaderCode = sa::ReadSPVFile("Passthrough.vert.spv");
		auto fshaderCode = sa::ReadSPVFile("Passthrough.frag.spv");

		sa::ShaderSet shaderSet = renderer.createShaderSet({ vshaderCode, fshaderCode });
		ResourceID pipeline = renderer.createGraphicsPipeline(renderProgram, 0, window.getCurrentExtent(), shaderSet);

		sa::Buffer vertexBuffer = renderer.createBuffer(sa::BufferType::VERTEX);
		vertexBuffer.write(box);

		sa::Buffer indexBuffer = renderer.createBuffer(sa::BufferType::INDEX);
		indexBuffer.write(boxIndices);


		ResourceID sceneDescriptorSet = shaderSet.allocateDescriptorSet(0);
		ResourceID objectDescriptorSet = shaderSet.allocateDescriptorSet(1);

		sa::Buffer sceneUniformBuffer = renderer.createBuffer(sa::BufferType::UNIFORM, sizeof(glm::mat4) * 2);
		sa::Buffer objectUniformBuffer = renderer.createBuffer(sa::BufferType::UNIFORM, sizeof(glm::mat4));


		glm::mat4 projection = glm::perspective(glm::radians(60.f), (float)window.getCurrentExtent().width / window.getCurrentExtent().height, 0.01f, 1000.f);
		projection[1][1] *= -1;

		sceneUniformBuffer
			<< glm::lookAt(glm::vec3(0, 0, 5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0))
			<< projection;


		glm::mat4 boxTransform(1);
		objectUniformBuffer << boxTransform;

		renderer.updateDescriptorSet(sceneDescriptorSet, 0, sceneUniformBuffer);
		renderer.updateDescriptorSet(objectDescriptorSet, 0, objectUniformBuffer);

		ResourceID sampler = renderer.createSampler();

		sa::Image image("Box.png");
		sa::Texture2D boxTexture(image, false);

		sa::Image image2("Colored_Character_Animation.png");
		sa::Texture2D characterTexture(image2, false);

		renderer.updateDescriptorSet(objectDescriptorSet, 1, { boxTexture, characterTexture }, sampler, 0);


		float timer = 0.f;

		while (window.isOpen()) {
			window.pollEvents();

			boxTransform = glm::rotate(boxTransform, glm::radians(1.f), glm::vec3(0, 1, 0));
			objectUniformBuffer.write(&boxTransform, sizeof(glm::mat4), 0);

			timer += 1.0f;

			sa::RenderContext context = window.beginFrame();
			if (context) {

				context.updateDescriptorSet(objectDescriptorSet, 0, objectUniformBuffer);

				context.beginRenderProgram(renderProgram, framebuffer, sa::SubpassContents::DIRECT);
				context.bindPipeline(pipeline);

				context.bindDescriptorSet(sceneDescriptorSet, pipeline);
				context.bindDescriptorSet(objectDescriptorSet, pipeline);
				
				context.bindVertexBuffers(0, { vertexBuffer });
				context.bindIndexBuffer(indexBuffer);

				context.pushConstant(pipeline, sa::ShaderStageFlagBits::VERTEX, timer);
				context.pushConstant(pipeline, sa::ShaderStageFlagBits::FRAGMENT, glm::vec4(1.f, 0.f, 0.5f, 1.f));
				context.pushConstant(pipeline, sa::ShaderStageFlagBits::FRAGMENT, 1, 32);


				context.drawIndexed(indexBuffer.getElementCount<uint32_t>(), 1);

				context.endRenderProgram(renderProgram);


				window.display();
			}

		}


	}
	catch (const std::exception& e) {
		SA_DEBUG_LOG_ERROR(e.what());
		
	}

	return 0;
}
