#include "RenderWindow.hpp"
#include "Camera.hpp"

#include <chrono>

struct VertexUV {
	glm::vec4 position;
	glm::vec2 uv;
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

struct SceneUbo {
	glm::mat4 view;
	glm::mat4 proj;
};

struct ObjectUbo {
	glm::mat4 model;
};


struct RenderPass {
	vr::TexturePtr depthImage;
	uint32_t renderPass;
	uint32_t frameBuffer;
};

RenderPass createSimpleRenderPass(vr::RenderWindow& window) {
	RenderPass renderPassStruct = {};

	auto renderer = vr::Renderer::get();

	renderPassStruct.depthImage = renderer->createDepthImage(window.getCurrentExtent());

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

	renderPassStruct.frameBuffer = renderer->createFramebuffer(renderPassStruct.renderPass, { renderPassStruct.depthImage });

	return std::move(renderPassStruct);
}

int main() {

	vr::RenderWindow window(1000, 600, "Minecraft");
	vr::Renderer* renderer = vr::Renderer::get();
	vr::ShaderPtr chunkVertexShader = renderer->createShader("ChunkVertexShader.spv", VK_SHADER_STAGE_VERTEX_BIT);
	vr::ShaderPtr chunkFragmentShader = renderer->createShader("ChunkFragmentShader.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
	vr::ShaderSet chunkShaderSet = renderer->createShaderSet(chunkVertexShader, chunkFragmentShader);

	vr::DescriptorSetPtr sceneDescriptorSet = chunkShaderSet.getDescriptorSet(0);
	Camera camera(window, sceneDescriptorSet, 0);

	auto startTime = std::chrono::high_resolution_clock::now();


	RenderPass renderPass = createSimpleRenderPass(window);

	uint32_t pipeline = renderer->createPipeline(chunkShaderSet, renderPass.renderPass, 0);

	ObjectUbo objectUbo = {};
	objectUbo.model = glm::mat4(1.0f);

	vr::BufferPtr objectUniformBuffer = renderer->createUniformBuffer(sizeof(ObjectUbo), &objectUbo);
	renderer->updateDescriptorSet(sceneDescriptorSet, 1, objectUniformBuffer, nullptr, nullptr, true);

	vr::BufferPtr vertexBuffer = renderer->createVertexBuffer(sizeof(quad), quad);
	vr::BufferPtr indexBuffer = renderer->createIndexBuffer(sizeof(quadIndices), quadIndices);

	vr::SamplerPtr sampler = renderer->createSampler(VK_FILTER_NEAREST);

	vr::Image image("Box.png");
	vr::TexturePtr texture = renderer->createTexture2D(image);
	renderer->updateDescriptorSet(sceneDescriptorSet, 2, nullptr, texture, sampler, true);
	

	while (window.isOpen()) {
		window.pollEvents();

		window.beginFrame();

		auto now = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration<float>(now - startTime);
		float dt = duration.count();

		camera.update(dt);

		renderer->beginRenderPass(renderPass.renderPass, renderPass.frameBuffer, VK_SUBPASS_CONTENTS_INLINE);

		renderer->bindPipeline(pipeline);

		renderer->bindVertexBuffer(vertexBuffer);
		renderer->bindIndexBuffer(indexBuffer);

		renderer->bindDescriptorSet(sceneDescriptorSet, pipeline);

		renderer->drawIndexed(6, 1);

		renderer->endRenderPass();

		window.endFrame();


	}

	return 0;
}