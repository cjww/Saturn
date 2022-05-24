#pragma once

#include "VulkanCore.hpp"

#include "Resources\ShaderSet.hpp"

namespace sa {

	class Attachment {
	private:
		vk::AttachmentDescription m_description;
	};

	class Subpass {
	private:
		vk::SubpassDescription m_description;
		std::vector<vk::AttachmentReference> m_colorAttachmentReferences;
		std::vector<vk::AttachmentReference> m_inputAttachmentReferences;
		std::vector<vk::AttachmentReference> m_resolveAttachmentReferences;
		std::optional<vk::AttachmentReference> m_depthAttachmentReference;
		
	public:
		Subpass() = default;
		
		void addAttachmentReference(uint32_t index, vk::ImageLayout layout);

		vk::SubpassDescription getDescription();

	};

	class RenderProgram {
	private:
		vk::Device m_device;
		vk::RenderPass m_renderPass;

		std::vector<Subpass> m_subpasses;
		
		std::vector<vk::SubpassDependency> m_dependencies;
		std::vector<vk::AttachmentDescription> m_attachments;

	public:
		RenderProgram();

		Subpass& newSubpass();
		void addAttachment(vk::ImageLayout intialLayout, vk::ImageLayout finalLayout, vk::Format format, vk::AttachmentLoadOp loadOp, vk::AttachmentStoreOp storeOp, vk::AttachmentLoadOp stencilLoadOp = vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp stencilStoreOp = vk::AttachmentStoreOp::eDontCare);

		void addSubpassDependency(vk::SubpassDependency dependency);

		void createPipeline(const ShaderSet& shaders);

		void create(VulkanCore* pCore);
		void destroy();


		void begin(CommandBufferSet* cmd, FramebufferSet* framebuffer, Color clearColor, Rect renderArea);
		void end(CommandBufferSet* cmd);

		vk::RenderPass getRenderPass() const;
	};
}