#define DEBUG_LOG

#include <iostream>


#include <Renderer.hpp>
#include <RenderWindow.hpp>
/*
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
*/

#include <chrono>
/*
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
*/


int main() {

#ifdef _WIN32
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	const int WIDTH = 1000, HEIGHT = 600;
	sa::RenderWindow window(WIDTH, HEIGHT, "Test Window");
	sa::Renderer& renderer = sa::Renderer::get();
	auto renderProgram = renderer.createRenderProgram()
		.end();
	

	auto framebuffer  = renderer.createFramebuffer(renderProgram)

	try {

		while (window.isOpen()) {
			window.pollEvents();

			if (window.beginFrame()) {


				window.display();
			}

		}
	}
	catch (std::exception e) {
		std::cout << e.what() << std::endl;
	}

	return 0;
}
