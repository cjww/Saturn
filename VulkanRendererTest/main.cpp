

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

#include <Renderer.hpp>
#include <RenderWindow.hpp>
#include <Tools\Logger.hpp>

#include <chrono>
#include <array>
#include <thread>

struct Vertex {
    glm::vec4 pos;
};

struct Scene {
    glm::mat4 view;
    glm::mat4 projection;
};


int main() {
    using namespace sa;

    srand(time(NULL));

    RenderWindow window(500, 500, "Test");

    auto& renderer = Renderer::get();

    DynamicTexture2D depthTexture = DynamicTexture2D(TextureTypeFlagBits::DEPTH_ATTACHMENT, window.getCurrentExtent());
    DynamicTexture2D colorTexture = DynamicTexture2D(TextureTypeFlagBits::COLOR_ATTACHMENT | TextureTypeFlagBits::SAMPLED, window.getCurrentExtent());

    ResourceID renderProgram = renderer.createRenderProgram()
        .addColorAttachment(true, colorTexture)
        .addDepthAttachment(depthTexture)
        .beginSubpass()
        .addAttachmentReference(0, SubpassAttachmentUsage::ColorTarget)
        .addAttachmentReference(1, SubpassAttachmentUsage::DepthTarget)
        .endSubpass()
        .beginSubpass()
        .addAttachmentReference(0, SubpassAttachmentUsage::ColorTarget)
        .endSubpass()
        .end();




    ResourceID imguiRenderProgram = renderer.createRenderProgram()
        .addColorAttachment(true, colorTexture)
        .beginSubpass()
        .addAttachmentReference(0, SubpassAttachmentUsage::ColorTarget)
        .endSubpass()
        .end();

    
    ResourceID swapchainRenderProgram = renderer.createRenderProgram()
        .addSwapchainAttachment(window.getSwapchainID())
        .beginSubpass()
        .addAttachmentReference(0, SubpassAttachmentUsage::ColorTarget)
        .endSubpass()
        .addColorDependency(SA_SUBPASS_EXTERNAL, 0) // wait for color attachment write of previous renderprogram before starting this subpass 0
        .end();

    renderer.initImGui(window, renderProgram, 1);
    ResourceID imGuiFramebuffer = renderer.createFramebuffer(imguiRenderProgram, { colorTexture });


    ResourceID framebuffer = renderer.createFramebuffer(renderProgram, { colorTexture, depthTexture });
    std::vector<sa::Texture> textures = {};
    ResourceID framebuffer2 = renderer.createSwapchainFramebuffer(swapchainRenderProgram, window.getSwapchainID(), textures);


    ResourceID pipeline = renderer.createGraphicsPipeline(renderProgram, 0, window.getCurrentExtent(), 
        "BareBones.vert.spv", "BareBones.frag.spv");
    //ResourceID pipeline = renderer.createGraphicsPipeline(renderProgram, 0, window.getCurrentExtent(), "BareBones.vert.spv", "BareBones.frag.spv");
    ResourceID pipeline2 = renderer.createGraphicsPipeline(swapchainRenderProgram, 0, window.getCurrentExtent(), 
        "PostProcess.vert.spv", "PostProcess.frag.spv");

    ResourceID descriptorSet2 = renderer.allocateDescriptorSet(pipeline2, 0);
    ResourceID sampler = renderer.createSampler(FilterMode::LINEAR);
    //renderer.updateDescriptorSet(descriptorSet2, 0, colorTexture, sampler);
    

    Buffer vertexBuffer = renderer.createBuffer(BufferType::VERTEX);
    std::vector<Vertex> vertices = {
        {glm::vec4(0.0f, -0.5f, 0.0f, 1.0f)},
        {glm::vec4(0.5f, 0.5f, 0.0f, 1.0f)},
        {glm::vec4(-0.5f, 0.5f, 0.0f, 1.0f)},
    };
    vertexBuffer.write(vertices);


    Scene scene = {};
    scene.view = glm::lookAt(glm::vec3( 0.f, 0.f, 1.f ), glm::vec3( 0.f, 0.f, 0.f ), glm::vec3( 0.f, 1.f, 0.f ));
    scene.projection = glm::perspective(glm::radians(120.f), window.getCurrentExtent().width / (float)window.getCurrentExtent().height, 0.1f, 1000.f);
    Buffer uniformbuffer = renderer.createBuffer(BufferType::UNIFORM, sizeof(Scene), &scene);

    ResourceID descriptorSet = renderer.allocateDescriptorSet(pipeline, 0);
    renderer.updateDescriptorSet(descriptorSet, 0, uniformbuffer);

    std::vector<glm::mat4> objects(100, glm::mat4(1));
    for (int i = 0; i < objects.size(); i++) {
        objects[i] = glm::translate(glm::mat4(1), glm::vec3((rand() % 10) - 5, (rand() % 10) - 5, -i));
    }

    auto now = std::chrono::high_resolution_clock::now();
    
    while (window.isOpen()) {
        window.pollEvents();
      
        renderer.newImGuiFrame();

        float dt = std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - now).count();
        now = std::chrono::high_resolution_clock::now();
        
        static float timer = 0.0f;
        timer += dt;
        if (timer > 0.5f) {
            SA_DEBUG_LOG_INFO("FPS: ", 1.f / dt, ", Frame Time: ", dt * 1000);
            timer = 0.f;
        }

        //window.setWindowTitle("FPS:" + std::to_string(1.f / dt));
        scene.view = glm::translate(scene.view, glm::vec3(0, 0, 1) * dt);
        if (scene.view[3].z > 10) {
            scene.view[3].z = 0;
        }
        uniformbuffer.write(scene);
        /*
        */
        

        RenderContext context = window.beginFrame();
        

        /*
        context = renderer.createContext()

        context.begin() // starts commandbuffer recording

        ... commands

        context.end(); // end commandbuffer recording

        //context.copyImage(colorTexture, target)
        //context.copyImage(colorTexture, window.getSwapchainID());
        
        //context.submit(); // submit recorded comandbuffer , increments buffer index

        window.display(context) // get next swapchain image, calls submit, present and synchronize 

        */
        
        if (context) {

            context.updateDescriptorSet(descriptorSet2, 0, colorTexture.getTexture(), sampler);
            context.updateDescriptorSet(descriptorSet, 0, uniformbuffer);
            
            context.beginRenderProgram(renderProgram, framebuffer, SubpassContents::DIRECT);
            
            context.bindVertexBuffers(0, { vertexBuffer });
            context.bindPipeline(pipeline);
            context.bindDescriptorSet(descriptorSet, pipeline);
            for (auto& mat : objects) {
                context.pushConstant(pipeline, ShaderStageFlagBits::VERTEX, mat);

                //context.draw(10000000, 1);
                context.draw(10000000, 1);

            }
            
            context.nextSubpass(SubpassContents::DIRECT);
            
            context.renderImGuiFrame();

            context.endRenderProgram(renderProgram);
            
            //context.barrierColorAttachment(colorTexture);
            context.beginRenderProgram(swapchainRenderProgram, framebuffer2, SubpassContents::DIRECT);

            context.bindPipeline(pipeline2);
            context.bindDescriptorSet(descriptorSet2, pipeline2);
            context.draw(6, 1);

            context.endRenderProgram(swapchainRenderProgram);

            window.display();

            depthTexture.swap();
            colorTexture.swap();
        
        }

    }
    

	return 0;
}

//class CameraController {
//private:
//	sa::RenderWindow& window;
//	glm::vec3 viewForward;
//	glm::vec3 viewPos;
//	glm::vec2 lastMousePos;
//
//	glm::mat4 mat = glm::mat4(1);
//	bool escapePressed;
//
//public:
//	float speed = 10.f;
//	float sensitivty = 1.f;
//		
//	bool mouseLocked;
//
//	CameraController(sa::RenderWindow& window) : window(window) {
//		viewForward = glm::vec3(0, 0, 1);
//		viewPos = glm::vec3(0.f);
//		
//		mouseLocked = true;
//		window.setCursorDisabled(true);
//	};
//
//	glm::mat4 getView(float dt) {
//		float hori = window.getKey(sa::Key::D) - window.getKey(sa::Key::A);
//		float vert = window.getKey(sa::Key::W) - window.getKey(sa::Key::S);
//
//	  	glm::vec2 mPos = { window.getCursorPosition().x, window.getCursorPosition().y };
//		glm::vec2 center = glm::vec2((float)window.getCurrentExtent().width / 2, (float)window.getCurrentExtent().height / 2);
//		glm::vec2 diff = mPos - center;
//		
//		float up = window.getKey(sa::Key::SPACE) - window.getKey(sa::Key::LEFT_CONTROL);
//		bool sprint = window.getKey(sa::Key::LEFT_SHIFT);
//		
//
//		if (sa::Window::IsGamepad(sa::Joystick::JOYSTICK_1)) {
//			sa::GamepadState state = sa::Window::GetGamepadState(sa::Joystick::JOYSTICK_1);
//
//			hori = state.getAxis(sa::GamepadAxis::LEFT_X);
//			vert = state.getAxis(sa::GamepadAxis::LEFT_Y) * -1.f;
//
//			diff.x = state.getAxis(sa::GamepadAxis::RIGHT_X);
//			diff.y = state.getAxis(sa::GamepadAxis::RIGHT_Y);
//
//			up = state.getAxis(sa::GamepadAxis::LEFT_TRIGGER) - state.getAxis(sa::GamepadAxis::RIGHT_TRIGGER);
//
//			sprint = state.getButton(sa::GamepadButton::RIGHT_BUMPER);
//
//		}
//
//		if (mouseLocked) {
//			window.setCursorPosition({ center.x, center.y });
//		}
//		else {
//			diff = glm::vec2(0, 0);
//		}
//
//		if (window.getKey(sa::Key::ESCAPE) && !escapePressed) {
//			mouseLocked = !mouseLocked;
//			window.setCursorDisabled(mouseLocked);
//		}
//		escapePressed = window.getKey(sa::Key::ESCAPE);
//
//
//		viewForward = glm::vec3(glm::vec4(viewForward, 0.0f) * glm::rotate(diff.x * dt * sensitivty, glm::vec3(0, 1, 0)));
//
//		glm::vec3 right = glm::cross(viewForward, glm::vec3(0, 1, 0));
//		viewForward = glm::vec3(glm::vec4(viewForward, 0.0f) * glm::rotate(diff.y * dt * sensitivty, right));
//
//
//		float finalSpeed = speed;
//		if (sprint) {
//			finalSpeed *= 2;
//		}
//
//		viewPos += right * hori * dt * finalSpeed;
//		viewPos += viewForward * vert * dt * finalSpeed;
//		viewPos += glm::vec3(0, 1, 0) * up * dt * finalSpeed;
//
//		
//		return glm::lookAt(viewPos, viewPos + viewForward, glm::vec3(0, 1, 0));
//	}
//	/*
//	glm::mat4 getViewRayTracing(float dt) {
//		int hori = window.getKey(sa::Key::D) - window.getKey(sa::Key::A);
//		int vert = window.getKey(sa::Key::W) - window.getKey(sa::Key::S);
//
//		glm::vec2 mPos = { window.getCursorPosition().x, window.getCursorPosition().y };
//		glm::vec2 center = glm::vec2((float)window.getCurrentExtent().width / 2, (float)window.getCurrentExtent().height / 2);
//		glm::vec2 diff = mPos - center;
//		if (mouseLocked) {
//			window.setCursorPosition({ center.x, center.y });
//		}
//		else {
//			diff = glm::vec2(0, 0);
//		}
//
//		if (window.getKey(GLFW_KEY_ESCAPE) && !escapePressed) {
//			mouseLocked = !mouseLocked;
//			window.setHideCursor(mouseLocked);
//		}
//		escapePressed = window.getKey(GLFW_KEY_ESCAPE);
//		
//		int up = glfwGetKey(window.getWindowHandle(), GLFW_KEY_SPACE) - glfwGetKey(window.getWindowHandle(), GLFW_KEY_LEFT_CONTROL);
//		bool sprint = glfwGetKey(window.getWindowHandle(), GLFW_KEY_LEFT_SHIFT);
//
//		mat *= glm::rotate(diff.x * dt * sensitivty, glm::vec3(0, 1, 0));
//
//		glm::vec3 right = glm::cross(viewForward, glm::vec3(0, 1, 0));
//		mat *= glm::rotate(-diff.y * dt * sensitivty, right);
//
//
//		float finalSpeed = speed;
//		if (sprint) {
//			finalSpeed *= 2;
//		}
//
//		mat = glm::translate(mat, -right * (float)hori * dt * finalSpeed);
//		mat = glm::translate(mat, viewForward * (float)vert * dt * finalSpeed);
//		mat = glm::translate(mat, glm::vec3(0, 1, 0) * (float)up * dt * finalSpeed);
//
//		return mat;
//
//	}
//	*/
//
//	glm::mat4 getProjection(float fovDegrees) {
//		auto projection = glm::perspective(glm::radians(fovDegrees), (float)window.getCurrentExtent().width / window.getCurrentExtent().height, 0.01f, 1000.0f);
//		projection[1][1] *= -1;
//		return projection;
//	}
//};
///*
//
//class FrameTimer {
//private:
//	std::chrono::steady_clock::time_point m_lastTime;
//public:
//	FrameTimer() {
//		m_lastTime = std::chrono::high_resolution_clock::now();
//	}
//	// return time since last call in seconds
//	float getDeltaTime() {
//		// Time calculations
//		auto now = std::chrono::high_resolution_clock::now();
//		auto dtDuration = std::chrono::duration<float, std::ratio<1, 1>>(now - m_lastTime);
//		m_lastTime = now;
//		float dt = dtDuration.count();
//		return dt;
//	}
//};
//
//struct RenderPass {
//	vr::Texture* depthImage;
//	uint32_t renderPass;
//	uint32_t frameBuffer;
//};
//
//struct Vertex {
//	glm::vec4 position;
//};
//
//struct VertexUV {
//	glm::vec4 position;
//	glm::vec2 uv;
//};
//
//struct Particle {
//	glm::mat4 mat;
//};
//
//struct UBO {
//	glm::mat4 view;
//	glm::mat4 projection;
//	int matrixCount;
//};
//
//struct ConfigBuffer {
//	glm::mat4 transform;
//	int matrixCount;
//};
//
//struct SceneUbo {
//	glm::mat4 view;
//	glm::mat4 proj;
//};
//
//struct ObjectUbo {
//	glm::mat4 model;
//};
//
//
//Vertex box[8] = {
//	// forward
//	{ { -1.0f, 1.0f, 1.0f, 1.0f } },
//	{ { 1.0f, 1.0f, 1.0f, 1.0f } },
//	{ { -1.0f, -1.0f, 1.0f, 1.0f } },
//	{ { 1.0f, -1.0f, 1.0f, 1.0f } },
//	// back
//	{ { -1.0f, 1.0f, -1.0f, 1.0f } },
//	{ { 1.0f, 1.0f, -1.0f, 1.0f } },
//	{ { -1.0f, -1.0f, -1.0f, 1.0f } },
//	{ { 1.0f, -1.0f, -1.0f, 1.0f } }
//};
//
//uint32_t boxIndices[] = {
//	// Forward
//	0, 1, 2, 1, 3, 2,
//	// Right
//	1, 5, 3, 5, 7, 3,
//	// Back
//	5, 4, 7, 4, 6, 7,
//	// Left
//	4, 0, 6, 0, 2, 6,
//	// Top
//	4, 5, 0, 5, 1, 0,
//	// Bottom
//	7, 6, 3, 6, 2, 3
//};
//
//VertexUV quad[] = {
//	{ glm::vec4(-0.5f, 0.5f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f) },
//	{ glm::vec4(0.5f, 0.5f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f) },
//	{ glm::vec4(0.5f, -0.5f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f) },
//	{ glm::vec4(-0.5f, -0.5f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f) }
//};
//
//uint32_t quadIndices[] = {
//	0, 1, 3,
//	1, 2, 3
//};
//*/
//
//struct UBO {
//	glm::mat4 view;
//	glm::mat4 projection;
//};
//
//struct PushConstant {
//	glm::mat4 world;
//};
//
//struct Vertex{
//	float position[4];
//};
//
//struct VertexColorUV {
//	float position[4];
//	float color[4];
//	float uv[2];
//};
//
//
//std::array<VertexColorUV, 4> quad = {
//	VertexColorUV{ { -0.5f, -0.5f, 0.0f, 1.0f },{ 1.0f, 0.0f, 0.0f, 1.0f }, {0.0f, 1.0f} },
//	VertexColorUV{ { 0.5f, -0.5f, 0.0f, 1.0f },	{ 0.0f, 1.0f, 0.0f, 1.0f }, {1.0f, 1.0f} },
//	VertexColorUV{ { 0.5f, 0.5f, 0.0f, 1.0f },	{ 0.0f, 0.0f, 1.0f, 1.0f }, {1.0f, 0.0f} },
//	VertexColorUV{ { -0.5f, 0.5f, 0.0f, 1.0f },	{ 1.0f, 1.0f, 1.0f, 1.0f }, {0.0f, 0.0f} }
//};
//
//std::array<uint32_t, 6> quadIndices = {
//	0, 1, 3,
//	1, 2, 3
//};
//
//std::array<VertexColorUV, 8> box = {
//	// forward
//	VertexColorUV{ { -1.0f, 1.0f, 1.0f, 1.0f }, { 1, 1, 1, 1 }, { 0, 1 } },
//	VertexColorUV{ { 1.0f, 1.0f, 1.0f, 1.0f }, { 1, 1, 1, 1 }, { 1, 1 } },
//	VertexColorUV{ { -1.0f, -1.0f, 1.0f, 1.0f }, { 1, 1, 1, 1 }, { 0, 0 } },
//	VertexColorUV{ { 1.0f, -1.0f, 1.0f, 1.0f }, { 1, 1, 1, 1 }, { 1, 0 } },
//	// back
//	VertexColorUV{ { -1.0f, 1.0f, -1.0f, 1.0f }, { 1, 1, 1, 1 }, { 0, 1 } },
//	VertexColorUV{ { 1.0f, 1.0f, -1.0f, 1.0f }, { 1, 1, 1, 1 }, { 1, 1 } },
//	VertexColorUV{ { -1.0f, -1.0f, -1.0f, 1.0f }, { 1, 1, 1, 1 }, { 0, 0 } },
//	VertexColorUV{ { 1.0f, -1.0f, -1.0f, 1.0f }, { 1, 1, 1, 1 }, { 1, 0 } }
//};
//
//std::array<uint32_t, 36> boxIndices = {
//	// Forward
//	0, 1, 2, 1, 3, 2,
//	// Right
//	1, 5, 3, 5, 7, 3,
//	// Back
//	5, 4, 7, 4, 6, 7,
//	// Left
//	4, 0, 6, 0, 2, 6,
//	// Top
//	4, 5, 0, 5, 1, 0,
//	// Bottom
//	7, 6, 3, 6, 2, 3
//};
//
//void voxelTest(sa::RenderWindow& window) {
//	sa::Renderer& renderer = sa::Renderer::get();
//
//	sa::Texture2D depthTexture = renderer.createTexture2D(sa::TextureTypeFlagBits::DEPTH_ATTACHMENT, window.getCurrentExtent());
//
//	ResourceID renderProgram = renderer.createRenderProgram()
//		.addSwapchainAttachment(window.getSwapchainID())
//		.addDepthAttachment(depthTexture)
//		.beginSubpass()
//			.addAttachmentReference(0, sa::SubpassAttachmentUsage::ColorTarget)
//			.addAttachmentReference(1, sa::SubpassAttachmentUsage::DepthTarget)
//		.endSubpass()
//		.end();
//
//	ResourceID framebuffer = renderer.createSwapchainFramebuffer(renderProgram, window.getSwapchainID(), { depthTexture });
//
//	sa::PipelineSettings settings = {};
//	settings.polygonMode = sa::PolygonMode::FILL;
//	settings.cullMode = sa::CullModeFlagBits::NONE;
//	settings.topology = sa::Topology::TRIANGLE_STRIP;
//
//	ResourceID pipeline = renderer.createGraphicsPipeline(renderProgram, 0, window.getCurrentExtent(),
//		"voxelPipeline.vert.spv", "voxelPipeline.geom.spv", "voxelPipeline.frag.spv", settings);
//
//	std::vector<Vertex> line = { 
//		{ -0.5f, 0.5f, 0.f, 1.f },
//		{ 0.5f, 0.5f, 0.f, 1.f },
//		{ 0.f, -0.5f, 0.f, 1.f }
//
//	};
//	sa::Buffer vertexBuffer = renderer.createBuffer(sa::BufferType::VERTEX, line.size() * sizeof(Vertex), line.data());
//
//	/*
//	*/
//	sa::Extent3D extent = { 64, 64, 64 };
//	sa::Texture3D tex3D = renderer.createTexture3D(sa::TextureTypeFlagBits::STORAGE | sa::TextureTypeFlagBits::SAMPLED, extent, sa::FormatPrecisionFlagBits::e8Bit, sa::FormatDimensionFlagBits::e4, sa::FormatTypeFlagBits::ANY_TYPE);
//	
//	sa::Texture2D storageTexture = renderer.createTexture2D(sa::TextureTypeFlagBits::STORAGE | sa::TextureTypeFlagBits::SAMPLED, { 128, 128 },
//		sa::FormatPrecisionFlagBits::ANY_PRECISION,
//		sa::FormatDimensionFlagBits::e4,
//		sa::FormatTypeFlagBits::ANY_TYPE);
//
//
//	ResourceID sampler = renderer.createSampler();
//
//	ResourceID descriptorSet0 = renderer.allocateDescriptorSet(pipeline, 0);
//	renderer.updateDescriptorSet(descriptorSet0, 0, storageTexture);
//
//	renderer.updateDescriptorSet(descriptorSet0, 1, tex3D);
//
//	renderer.updateDescriptorSet(descriptorSet0, 2, tex3D, sampler);
//
//	bool transitioned = false;
//
//	while (window.isOpen()) {
//		window.pollEvents();
//
//		sa::RenderContext context = window.beginFrame();
//		if (context) {
//			
//			if (!transitioned) {
//				context.transitionTexture(tex3D, sa::Transition::NONE, sa::Transition::FRAGMENT_SHADER_WRITE);
//				context.transitionTexture(storageTexture, sa::Transition::NONE, sa::Transition::FRAGMENT_SHADER_WRITE);
//				transitioned = true;
//			}
//
//			context.beginRenderProgram(renderProgram, framebuffer, sa::SubpassContents::DIRECT);
//			context.bindPipeline(pipeline);
//
//			context.bindDescriptorSet(descriptorSet0, pipeline);
//			context.bindVertexBuffers(0, { vertexBuffer });
//			context.draw(vertexBuffer.getElementCount<Vertex>(), 1);
//
//			context.endRenderProgram(renderProgram);
//
//		}
//		window.display();
//
//
//	}
//
//}
//
//void imguiTest(sa::RenderWindow& window) {
//	sa::Renderer& renderer = sa::Renderer::get();
//
//	// Textures for rendering
//	sa::Texture2D colorTexture = renderer.createTexture2D(sa::TextureTypeFlagBits::COLOR_ATTACHMENT, window.getCurrentExtent(), 8);
//	sa::Texture2D depthTexture = renderer.createTexture2D(sa::TextureTypeFlagBits::DEPTH_ATTACHMENT, window.getCurrentExtent(), 8);
//	sa::Texture2D resolveTexture = renderer.createTexture2D(sa::TextureTypeFlagBits::COLOR_ATTACHMENT | sa::TextureTypeFlagBits::SAMPLED, window.getCurrentExtent());
//
//	// Render programs
//	// One for the main rendering
//	ResourceID renderProgram = renderer.createRenderProgram()
//		.addColorAttachment(false, colorTexture)
//		.addDepthAttachment(depthTexture)
//		.addColorAttachment(true, resolveTexture)
//		.beginSubpass()
//			.addAttachmentReference(0, sa::SubpassAttachmentUsage::ColorTarget)
//			.addAttachmentReference(1, sa::SubpassAttachmentUsage::DepthTarget)
//			.addAttachmentReference(2, sa::SubpassAttachmentUsage::Resolve)
//		.endSubpass()
//		.end();
//	// one to render onto imgui window
//	ResourceID imguiProgram = renderer.createRenderProgram()
//		.addSwapchainAttachment(window.getSwapchainID())
//		.beginSubpass()
//			.addAttachmentReference(0, sa::SubpassAttachmentUsage::ColorTarget)
//		.endSubpass()
//		.end();
//
//	// corresponding framebuffers to the programs
//	ResourceID framebuffer = renderer.createFramebuffer(renderProgram, { colorTexture, depthTexture, resolveTexture });
//	ResourceID imguiFramebuffer = renderer.createSwapchainFramebuffer(imguiProgram, window.getSwapchainID(), { });
//
//	// main pipeline
//	ResourceID pipeline = renderer.createGraphicsPipeline(renderProgram, 0, window.getCurrentExtent(), "ForwardShader.vert.spv", "ForwardShader.frag.spv");
//	
//	sa::PipelineSettings pipelineSettings = {};
//	pipelineSettings.cullMode = sa::CullModeFlagBits::FRONT;
//	pipelineSettings.depthTestEnabled = false;
//	ResourceID skyboxPipeline = renderer.createGraphicsPipeline(renderProgram, 0, window.getCurrentExtent(), "Skybox.vert.spv", "Skybox.frag.spv", pipelineSettings);
//
//	renderer.initImGui(window, imguiProgram, 0);
//
//	// Some test textures
//	sa::Image image("Box.png");
//	sa::Texture2D boxTexture = renderer.createTexture2D(image, true);
//	
//	sa::Image characterImage("Colored_Character_Animation.png");
//	sa::Texture2D characterTexture = renderer.createTexture2D(characterImage, true);
//
//	// structures
//	struct PushConstants {
//		glm::mat4 worldMat;
//		int textureIndex;
//	};
//	struct Object {
//		sa::Buffer vertexBuffer;
//		sa::Buffer indexBuffer;
//		PushConstants pc;
//	};
//
//	// Test Object
//	Object object;
//	object.vertexBuffer = renderer.createBuffer(sa::BufferType::VERTEX);
//	object.vertexBuffer.write(box);
//	object.vertexBuffer << quad;
//
//	object.indexBuffer = renderer.createBuffer(sa::BufferType::INDEX);
//	object.indexBuffer.write(boxIndices);
//	object.indexBuffer << quadIndices;
//
//	object.pc.worldMat = glm::mat4(1);
//	object.pc.textureIndex = 0;
//
//	CameraController camera(window);
//
//	// update to GPU
//	ResourceID descriptorSet = renderer.allocateDescriptorSet(pipeline, 0);
//
//	UBO ubo = {
//		camera.getView(0.1),
//		camera.getProjection(90)
//	};
//	sa::Buffer uniformBuffer = renderer.createBuffer(sa::BufferType::UNIFORM);
//	uniformBuffer.write(ubo);
//
//	ResourceID sampler = renderer.createSampler();
//
//	renderer.updateDescriptorSet(descriptorSet, 0, uniformBuffer);
//	
//	renderer.updateDescriptorSet(descriptorSet, 1, sampler);
//	renderer.updateDescriptorSet(descriptorSet, 2, { boxTexture, characterTexture });
//
//	// Skybox test
//	ResourceID skyboxDescriptorSet = renderer.allocateDescriptorSet(skyboxPipeline, 0);
//
//	sa::Image back("skybox/back.jpg");
//	sa::Image bottom("skybox/bottom.jpg");
//	sa::Image front("skybox/front.jpg");
//	sa::Image left("skybox/left.jpg");
//	sa::Image right("skybox/right.jpg");
//	sa::Image top("skybox/top.jpg");
//	sa::TextureCube cubeMap1 = renderer.createTextureCube({ right, left, top, bottom, front, back}, false);
//	
//	sa::Image skybox("cubemap_skybox.png");
//	sa::TextureCube cubeMap2 = renderer.createTextureCube(skybox, false);
//
//
//	renderer.updateDescriptorSet(skyboxDescriptorSet, 0, uniformBuffer);
//	renderer.updateDescriptorSet(skyboxDescriptorSet, 1, cubeMap1, sampler);
//
//
//	sa::Buffer indirectCountBuffer = renderer.createBuffer(sa::BufferType::INDIRECT);
//	indirectCountBuffer << 2U;
//
//	sa::Buffer indirectBuffer = renderer.createBuffer(sa::BufferType::INDIRECT, sizeof(sa::DrawIndexedIndirectCommand) * 2);
//
//	sa::DrawIndexedIndirectCommand cmd = {};
//	cmd.firstIndex = 0;
//	cmd.indexCount = boxIndices.size();
//	cmd.firstInstance = 0;
//	cmd.instanceCount = 1;
//	cmd.vertexOffset = 0;
//	indirectBuffer << cmd;
//
//	cmd.firstIndex = boxIndices.size();
//	cmd.indexCount = quadIndices.size();
//	cmd.firstInstance = 1;
//	cmd.instanceCount = 2;
//	cmd.vertexOffset = box.size();
//	indirectBuffer << cmd;
//
//	// Timing
//	auto now = std::chrono::high_resolution_clock::now();
//
//	renderer.setClearColor(renderProgram, sa::Color{ 1, 0, 1 , 1});
//
//	while (window.isOpen()) {
//		window.pollEvents();
//		
//		renderer.newImGuiFrame();
//
//		ImGui::DockSpaceOverViewport();
//
//		// Timing
//		float dt = std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - now).count();
//		now = std::chrono::high_resolution_clock::now();
//
//		ubo.view = camera.getView(dt);
//		glm::vec4 viewDir = ubo.view[2];
//		
//		uniformBuffer.write(ubo);
//		if (ImGui::Begin("Settings")) {
//			ImGui::Text("Frame time: %f", dt);
//			ImGui::Text("FPS: %f", 1.0f/dt);
//			ImGui::Text("View dir: (%f, %f, %f, %f)", viewDir.x, viewDir.y, viewDir.z, viewDir.w);
//			ImGui::End();
//		}
//
//		if (ImGui::Begin("Scene")) {
//			ImGui::Image(resolveTexture, resolveTexture.getExtent());
//			ImGui::End();
//		}
//
//		sa::RenderContext context = window.beginFrame();
//		if (context) {
//
//			static bool secondType = false;
//			if (ImGui::Checkbox("Skybox type", &secondType)) {
//				if (secondType) {
//					renderer.updateDescriptorSet(skyboxDescriptorSet, 1, cubeMap2, sampler);
//				}
//				else {
//					renderer.updateDescriptorSet(skyboxDescriptorSet, 1, cubeMap1, sampler);
//				}
//
//			}
//			context.updateDescriptorSet(descriptorSet, 0, uniformBuffer);
//
//			context.beginRenderProgram(renderProgram, framebuffer, sa::SubpassContents::DIRECT);
//
//			context.bindPipeline(skyboxPipeline);
//			context.bindDescriptorSet(skyboxDescriptorSet, skyboxPipeline);
//			context.draw(36, 1);
//
//			context.bindPipeline(pipeline);
//			context.bindDescriptorSet(descriptorSet, pipeline);
//			context.bindVertexBuffers(0, { object.vertexBuffer });
//			context.bindIndexBuffer(object.indexBuffer);
//
//			glm::mat4 mat = glm::translate(glm::mat4(1), { 2, 0, 0 });
//			struct PC {
//				glm::mat4 mat[4];
//			} pc;
//			pc.mat[0] = object.pc.worldMat;
//			pc.mat[1] = mat;
//			mat = glm::translate(mat, { 2, 0, 0 });
//			pc.mat[2] = mat;
//
//			context.pushConstant(pipeline, sa::ShaderStageFlagBits::VERTEX, pc);
//			
//			//context.drawIndexed(object.indexBuffer.getElementCount<uint32_t>(), 1);
//			context.drawIndexedIndirect(indirectBuffer, 0, 2, sizeof(sa::DrawIndexedIndirectCommand));
//
//			context.endRenderProgram(renderProgram);
//			context.beginRenderProgram(imguiProgram, imguiFramebuffer, sa::SubpassContents::DIRECT);
//			
//			context.renderImGuiFrame();
//			
//			context.endRenderProgram(imguiProgram);
//			
//			window.display();
//		
//		}
//	}
//}
//
//void deffered(sa::RenderWindow& window) {
//	sa::Renderer& renderer = sa::Renderer::get();
//
//	// FIRST PASS
//	sa::Texture2D colorTexture = renderer.createTexture2D(
//		sa::TextureTypeFlagBits::COLOR_ATTACHMENT |
//		sa::TextureTypeFlagBits::INPUT_ATTACHMENT,
//		window.getCurrentExtent());
//
//	sa::Texture2D depthTexture = renderer.createTexture2D(
//		sa::TextureTypeFlagBits::DEPTH_ATTACHMENT,
//		window.getCurrentExtent());
//
//	sa::Texture2D positionsTexture = renderer.createTexture2D(
//		sa::TextureTypeFlagBits::COLOR_ATTACHMENT |
//		sa::TextureTypeFlagBits::INPUT_ATTACHMENT,
//		window.getCurrentExtent(),
//		sa::FormatPrecisionFlagBits::e32Bit,
//		sa::FormatDimensionFlagBits::e4,
//		sa::FormatTypeFlagBits::SFLOAT);
//
//	sa::Texture2D shadedTexture = renderer.createTexture2D(
//		sa::TextureTypeFlagBits::COLOR_ATTACHMENT |
//		sa::TextureTypeFlagBits::SAMPLED,
//		window.getCurrentExtent());
//
//	sa::Texture2D brightnessTexture = renderer.createTexture2D(
//		sa::TextureTypeFlagBits::COLOR_ATTACHMENT |
//		sa::TextureTypeFlagBits::SAMPLED,
//		window.getCurrentExtent());
//
//
//	auto defferedProgram = renderer.createRenderProgram()
//		.addColorAttachment(false, colorTexture) // 0 color
//		.addDepthAttachment() // 1 depth
//		.addColorAttachment(false, positionsTexture) // 2 positions
//		.addColorAttachment(true, shadedTexture) // 3 combine
//		.addColorAttachment(true, brightnessTexture) // 4 brightness
//		.beginSubpass()
//		.addAttachmentReference(0, sa::SubpassAttachmentUsage::ColorTarget)
//		.addAttachmentReference(1, sa::SubpassAttachmentUsage::DepthTarget)
//		.addAttachmentReference(2, sa::SubpassAttachmentUsage::ColorTarget)
//		.endSubpass()
//		.beginSubpass()
//		.addAttachmentReference(3, sa::SubpassAttachmentUsage::ColorTarget)
//		.addAttachmentReference(4, sa::SubpassAttachmentUsage::ColorTarget)
//		.addAttachmentReference(0, sa::SubpassAttachmentUsage::Input)
//		.addAttachmentReference(2, sa::SubpassAttachmentUsage::Input)
//		.endSubpass()
//		.end();
//
//
//	auto gFramebuffer = renderer.createFramebuffer(
//		defferedProgram,
//		{ colorTexture, depthTexture, positionsTexture, shadedTexture, brightnessTexture });
//
//	auto defferedPipeline = renderer.createGraphicsPipeline(
//		defferedProgram,
//		0,
//		window.getCurrentExtent(),
//		"TestShader.vert.spv",
//		"TestShader.frag.spv");
//
//	auto combinePipeline = renderer.createGraphicsPipeline(
//		defferedProgram,
//		1,
//		window.getCurrentExtent(),
//		"PostProcess.vert.spv",
//		"PostProcess.frag.spv");
//
//	ResourceID defferedDescriptorSet = renderer.allocateDescriptorSet(defferedPipeline, 0);
//
//
//	sa::Buffer sceneUniformBuffer = renderer.createBuffer(
//		sa::BufferType::UNIFORM);
//
//	CameraController camera(window);
//
//	UBO ubo = {};
//	ubo.view = glm::lookAt(glm::vec3{ 0, 0, 1 }, { 0, 0, 0 }, { 0, 1, 0 });
//	ubo.projection = camera.getProjection(90);
//
//	sceneUniformBuffer.write(ubo);
//	renderer.updateDescriptorSet(defferedDescriptorSet, 0, sceneUniformBuffer);
//
//
//	sa::Image boxImage("Box.png");
//	sa::Texture2D texture = renderer.createTexture2D(boxImage, false);
//	ResourceID sampler = renderer.createSampler(sa::FilterMode::LINEAR);
//	renderer.updateDescriptorSet(defferedDescriptorSet, 2, texture, sampler);
//
//
//	ResourceID combineDescriptorSet = renderer.allocateDescriptorSet(combinePipeline, 0);
//	renderer.updateDescriptorSet(combineDescriptorSet, 0, colorTexture);
//	renderer.updateDescriptorSet(combineDescriptorSet, 1, positionsTexture);
//
//	// SECOND PASS
//
//	auto mainRenderProgram = renderer.createRenderProgram()
//		.addSwapchainAttachment(window.getSwapchainID()) // 0 swapchain
//		.beginSubpass()
//		.addAttachmentReference(0, sa::SubpassAttachmentUsage::ColorTarget)
//		.endSubpass()
//		.end();
//
//	auto mainFramebuffer = renderer.createSwapchainFramebuffer(
//		mainRenderProgram,
//		window.getSwapchainID(),
//		{ });
//
//	auto mainPipeline = renderer.createGraphicsPipeline(
//		mainRenderProgram,
//		0,
//		window.getCurrentExtent(),
//		"PostProcess.vert.spv",
//		"MainShader.frag.spv");
//
//	auto mainDescriptorSet = renderer.allocateDescriptorSet(mainPipeline, 0);
//	renderer.updateDescriptorSet(mainDescriptorSet, 0, shadedTexture, sampler);
//
//	sa::Buffer vertexBuffer = renderer.createBuffer(
//		sa::BufferType::VERTEX, quad.size() * sizeof(VertexColorUV), quad.data());
//	sa::Buffer indexBuffer = renderer.createBuffer(
//		sa::BufferType::INDEX);
//	indexBuffer.write<uint32_t, 6>({
//		0, 1, 3,
//		1, 2, 3
//		});
//
//	sa::Buffer boxVertexBuffer = renderer.createBuffer(
//		sa::BufferType::VERTEX);
//	boxVertexBuffer.write(box);
//	sa::Buffer boxIndexBuffer = renderer.createBuffer(
//		sa::BufferType::INDEX);
//	boxIndexBuffer.write(boxIndices);
//
//
//
//
//	//Compute
//	ResourceID computePipeline = renderer.createComputePipeline("ComputeShader.comp.spv");
//	ResourceID computeDescriptorSet = renderer.allocateDescriptorSet(computePipeline, 0);
//
//	std::vector<glm::mat4> data(100000, glm::mat4(1));
//
//	sa::Buffer configBuffer = renderer.createBuffer(sa::BufferType::UNIFORM);
//	configBuffer.write(data.size());
//
//	sa::Buffer outputBuffer = renderer.createBuffer(sa::BufferType::STORAGE);
//	outputBuffer.write(data);
//
//	renderer.updateDescriptorSet(computeDescriptorSet, 0, configBuffer);
//	renderer.updateDescriptorSet(computeDescriptorSet, 2, outputBuffer);
//
//	// Compute Blur
//	ResourceID blurPipeline = renderer.createComputePipeline("BlurShader.comp.spv");
//	ResourceID blurDescriptorSet = renderer.allocateDescriptorSet(blurPipeline, 0);
//
//	sa::Extent extent = {
//		(uint32_t)(window.getCurrentExtent().width / 4.f),
//		(uint32_t)(window.getCurrentExtent().height / 4.f)
//	};
//	//sa::Extent extent = window.getCurrentExtent();
//	sa::Texture2D blurredTexture = renderer.createTexture2D(
//		sa::TextureTypeFlagBits::STORAGE | sa::TextureTypeFlagBits::SAMPLED,
//		extent,
//		sa::FormatPrecisionFlagBits::e32Bit, sa::FormatDimensionFlagBits::e4, sa::FormatTypeFlagBits::SFLOAT);
//
//	sa::Buffer blurConfigBuffer = renderer.createBuffer(
//		sa::BufferType::UNIFORM,
//		sizeof(extent), &extent);
//
//
//
//	renderer.updateDescriptorSet(blurDescriptorSet, 0, blurConfigBuffer);
//	renderer.updateDescriptorSet(blurDescriptorSet, 1, brightnessTexture, sampler);
//	renderer.updateDescriptorSet(blurDescriptorSet, 2, blurredTexture);
//
//	renderer.updateDescriptorSet(mainDescriptorSet, 1, blurredTexture, sampler);
//
//	sa::DirectContext tmp = renderer.createDirectContext();
//	tmp.begin(sa::ContextUsageFlagBits::SIMULTANEOUS_USE);
//	tmp.transitionTexture(blurredTexture, sa::Transition::NONE, sa::Transition::COMPUTE_SHADER_WRITE);
//	tmp.end();
//	tmp.submit();
//	tmp.destroy();
//
//	sa::SubContext subContext = renderer.createSubContext(gFramebuffer, defferedProgram, 1);
//	/*
//	subContext.preRecord([=](sa::RenderContext& context) {
//		context.bindPipeline(combinePipeline);
//		context.bindVertexBuffers(0, { vertexBuffer });
//		context.bindIndexBuffer(indexBuffer);
//		context.bindDescriptorSet(combineDescriptorSet, combinePipeline);
//		context.drawIndexed(6, 1);
//	}, sa::ContextUsageFlagBits::RENDER_PROGRAM_CONTINUE);
//	*/
//
//
//	
//	auto now = std::chrono::high_resolution_clock::now();
//	float dt = 0;
//	float timer = 0.0f;
//
//	while (window.isOpen()) {
//		window.pollEvents();
//
//		timer += dt;
//		ubo.view = camera.getView(dt);
//
//		window.setWindowTitle("FPS: " + std::to_string(1 / dt));
//
//		data = outputBuffer.getContent<glm::mat4>();
//
//
//		sa::RenderContext context = window.beginFrame();
//		if (context) {
//
//			
//			subContext.begin(sa::ContextUsageFlagBits::RENDER_PROGRAM_CONTINUE | sa::ContextUsageFlagBits::ONE_TIME_SUBMIT);
//			subContext.bindPipeline(combinePipeline);
//			subContext.bindVertexBuffers(0, { vertexBuffer });
//			subContext.bindIndexBuffer(indexBuffer);
//			subContext.bindDescriptorSet(combineDescriptorSet, combinePipeline);
//			subContext.drawIndexed(6, 1);
//			subContext.end();
//
//			sceneUniformBuffer.write(ubo);
//			context.updateDescriptorSet(defferedDescriptorSet, 0, sceneUniformBuffer);
//			context.updateDescriptorSet(defferedDescriptorSet, 1, outputBuffer);
//
//
//			context.transitionTexture(blurredTexture, sa::Transition::RENDER_PROGRAM_INPUT, sa::Transition::COMPUTE_SHADER_WRITE);
//
//
//			context.bindPipeline(computePipeline);
//			context.bindDescriptorSet(computeDescriptorSet, computePipeline);
//			context.pushConstant(computePipeline, sa::ShaderStageFlagBits::COMPUTE, 0, timer);
//
//			int groupcount = ((data.size()) / 256) + 1;
//			context.dispatch(groupcount, 1, 1);
//
//			context.beginRenderProgram(defferedProgram, gFramebuffer, sa::SubpassContents::DIRECT);
//
//			context.bindPipeline(defferedPipeline);
//			context.bindDescriptorSet(defferedDescriptorSet, defferedPipeline);
//			// Drawing
//			context.bindVertexBuffers(0, { boxVertexBuffer });
//			context.bindIndexBuffer(boxIndexBuffer);
//
//			context.drawIndexed(boxIndices.size(), data.size());
//
//			context.nextSubpass(sa::SubpassContents::SUB_CONTEXT);
//			/*
//			context.bindPipeline(combinePipeline);
//			context.bindVertexBuffers(0, { vertexBuffer });
//			context.bindIndexBuffer(indexBuffer);
//			context.bindDescriptorSet(combineDescriptorSet, combinePipeline);
//
//			context.drawIndexed(6, 1);
//			*/
//			context.executeSubContext(subContext);
//
//			context.endRenderProgram(defferedProgram);
//
//
//			context.bindPipeline(blurPipeline);
//			context.bindDescriptorSet(blurDescriptorSet, blurPipeline);
//			int groupcountX = (extent.width / 32) + 1;
//			int groupcountY = (extent.height / 32) + 1;
//			context.dispatch(groupcountX, groupcountY, 1);
//
//			//context.barrier(blurredTexture);
//
//			context.transitionTexture(blurredTexture, sa::Transition::COMPUTE_SHADER_WRITE, sa::Transition::RENDER_PROGRAM_INPUT);
//
//			context.beginRenderProgram(mainRenderProgram, mainFramebuffer, sa::SubpassContents::DIRECT);
//			context.bindPipeline(mainPipeline);
//			context.bindDescriptorSet(mainDescriptorSet, mainPipeline);
//			context.drawIndexed(6, 1);
//
//			context.endRenderProgram(mainRenderProgram);
//
//			window.display();
//		}
//
//		dt = std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - now).count();
//		now = std::chrono::high_resolution_clock::now();
//
//	}
//}
//
//void forward(sa::RenderWindow& window) {
//	sa::Renderer& renderer = sa::Renderer::get();
//
//	window.setResizeCallback([](sa::Extent newExtent) {
//		SA_DEBUG_LOG_INFO("CALLBACK: new Extent: ", newExtent.width, newExtent.height);
//	});
//
//	// FIRST PASS
//	sa::Texture2D colorTexture = renderer.createTexture2D(
//		sa::TextureTypeFlagBits::COLOR_ATTACHMENT,
//		window.getCurrentExtent(), window.getSwapchainID(), 8);
//
//	sa::Texture2D depthTexture = renderer.createTexture2D(
//		sa::TextureTypeFlagBits::DEPTH_ATTACHMENT,
//		window.getCurrentExtent(), 8);
//
//
//	auto mainRenderProgram = renderer.createRenderProgram()
//		.addSwapchainAttachment(window.getSwapchainID()) // 0 swapchain
//		.addColorAttachment(false, colorTexture) // 1 color
//		.addDepthAttachment(depthTexture) // 2 depth
//		.beginSubpass()
//			.addAttachmentReference(1, sa::SubpassAttachmentUsage::ColorTarget)
//			.addAttachmentReference(2, sa::SubpassAttachmentUsage::DepthTarget)
//			.addAttachmentReference(0, sa::SubpassAttachmentUsage::Resolve)
//		.endSubpass()
//		.end();
//
//
//	renderer.setClearColor(mainRenderProgram, { 0.2, 0.3, 0.3, 1 });
//
//	auto mainFramebuffer = renderer.createSwapchainFramebuffer(
//		window.getSwapchainID(),
//		mainRenderProgram,
//		{ colorTexture, depthTexture });
//
//	auto mainPipeline = renderer.createGraphicsPipeline(
//		mainRenderProgram,
//		0,
//		window.getCurrentExtent(),
//		"ForwardShader.vert.spv",
//		"ForwardShader.frag.spv");
//
//
//	ResourceID mainDescriptorSet = renderer.allocateDescriptorSet(mainPipeline, 0);
//
//	sa::Buffer uniformBuffer = renderer.createBuffer(
//		sa::BufferType::UNIFORM);
//
//	CameraController camera(window);
//
//	UBO ubo = {};
//	ubo.view = glm::lookAt(glm::vec3{ 0, 0, 1 }, { 0, 0, 0 }, { 0, 1, 0 });
//	ubo.projection = camera.getProjection(90);
//
//	uniformBuffer.write(ubo);
//	renderer.updateDescriptorSet(mainDescriptorSet, 0, uniformBuffer);
//
//
//	sa::Image image("Box.png");
//	
//	sa::Texture2D texture = renderer.createTexture2D(image, false);
//	ResourceID sampler = renderer.createSampler(sa::FilterMode::LINEAR);
//	renderer.updateDescriptorSet(mainDescriptorSet, 1, texture, sampler);
//
//
//	sa::Buffer vertexBuffer = renderer.createBuffer(
//		sa::BufferType::VERTEX, quad.size() * sizeof(VertexColorUV), quad.data());
//	sa::Buffer indexBuffer = renderer.createBuffer(
//		sa::BufferType::INDEX);
//	indexBuffer.write<uint32_t, 6>({
//		0, 1, 3,
//		1, 2, 3
//		});
//
//	sa::Buffer boxVertexBuffer = renderer.createBuffer(
//		sa::BufferType::VERTEX);
//	boxVertexBuffer.write(box);
//	sa::Buffer boxIndexBuffer = renderer.createBuffer(
//		sa::BufferType::INDEX);
//	boxIndexBuffer.write(boxIndices);
//
//	auto now = std::chrono::high_resolution_clock::now();
//	float dt = 0;
//	float timer = 0.0f;
//
//	//Objects
//	struct Object {
//		glm::mat4 worldMat;
//		sa::SubContext context;
//	};
//
//	std::vector<Object> objects;
//	const uint32_t objectsPerThread = 100;
//	std::vector<std::thread> threads(7);
//	std::vector<ResourceID> contextPools(threads.size());
//	for (int i = 0; i < threads.size(); i++) {
//		contextPools[i] = renderer.createContextPool();
//
//		for (int j = 0; j < objectsPerThread; j++) {
//			objects.emplace_back(Object{
//				glm::translate(glm::mat4(1), glm::vec3(j * 4, i * 4, 0)),
//				renderer.createSubContext(mainFramebuffer, mainRenderProgram, 0, contextPools[i])
//			});
//		}
//	}
//	
//	while (window.isOpen()) {
//		window.pollEvents();
//
//		timer += dt;
//		ubo.view = camera.getView(dt);
//
//		window.setWindowTitle("FPS: " + std::to_string(1 / dt));
//		sa::RenderContext context = window.beginFrame();
//		if (context) {
//
//			uniformBuffer.write(ubo);
//			context.updateDescriptorSet(mainDescriptorSet, 0, uniformBuffer);
//			
//			for (int i = 0; i < threads.size(); i++) {
//				std::thread& thread = threads.at(i);
//				thread = std::thread([=, &objects]() {
//					for (int j = 0; j < objectsPerThread; j++) {
//						using namespace std::chrono_literals;
//						Object& object = objects[(i * objectsPerThread) + j];
//						//std::this_thread::sleep_for(5ms); // expensive work
//						object.worldMat = glm::translate(object.worldMat, glm::vec3(0, 1 * dt, 0));
//
//						object.context.begin(sa::ContextUsageFlagBits::RENDER_PROGRAM_CONTINUE);
//						object.context.bindPipeline(mainPipeline);
//						object.context.bindDescriptorSet(mainDescriptorSet, mainPipeline);
//						object.context.bindVertexBuffers(0, { boxVertexBuffer });
//						object.context.bindIndexBuffer(boxIndexBuffer);
//						object.context.pushConstant(mainPipeline, sa::ShaderStageFlagBits::VERTEX, object.worldMat);
//						object.context.drawIndexed(36, 1);
//						object.context.end();
//
//					}
//				});
//			}
//
//
//			for (auto& thread : threads) {
//				thread.join();
//			}
//		
//			context.beginRenderProgram(mainRenderProgram, mainFramebuffer, sa::SubpassContents::SUB_CONTEXT);
//
//			for (Object& obj : objects) {
//				context.executeSubContext(obj.context);
//			}
//			
//			context.endRenderProgram(mainRenderProgram);
//
//			window.display();
//		}
//
//		dt = std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - now).count();
//		now = std::chrono::high_resolution_clock::now();
//
//	}
//}
//
//int main() {
//
//#ifdef _WIN32
//	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
//#endif
//
//
//	try {
//		const int WIDTH = 1400, HEIGHT = 800;
//		sa::RenderWindow window(WIDTH, HEIGHT, "Test Window");
//		
//		ResourceID crossHairCursor = sa::Window::CreateCursor(sa::StandardCursor::CROSSHAIR);
//		window.setCursor(crossHairCursor);
//
//		sa::Window::SetJoystickConnectedCallback([](sa::Joystick joystick, sa::ConnectionState state) {
//			if (state == sa::ConnectionState::CONNECTED) {
//				SA_DEBUG_LOG_INFO("Joystick", (int)joystick, "connected: ", sa::Window::GetJoystickName(joystick));
//			}
//			else {
//				SA_DEBUG_LOG_INFO("Joystick", (int)joystick, "disconnected");
//			}
//		});
//
//		//voxelTest(window);
//		imguiTest(window);
//		//deffered(window);
//		//forward(window);
//
//
//	}
//	catch (const std::exception& e) {
//		SA_DEBUG_LOG_ERROR(e.what());
//		
//	}
//
//	return 0;
//}
