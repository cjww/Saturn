#pragma once

#include "VulkanCore.hpp"

#include "Resources\Pipeline.hpp"
#include "Resources\FramebufferSet.hpp"

#include "structs.hpp"

namespace sa {


	class Subpass {
	private:
		vk::SubpassDescription m_description;
		std::vector<vk::AttachmentReference> m_colorAttachmentReferences;
		std::vector<vk::AttachmentReference> m_inputAttachmentReferences;
		std::vector<vk::AttachmentReference> m_resolveAttachmentReferences;
		std::optional<vk::AttachmentReference> m_depthAttachmentReference;
		
		vk::SampleCountFlagBits m_sampleCount;
	public:
		Subpass();
		
		void addColorAttachmentReference(uint32_t index, vk::ImageLayout layout);
		void addInputAttachmentReference(uint32_t index, vk::ImageLayout layout);
		void addResolveAttachmentReference(uint32_t index, vk::ImageLayout layout);
		void setDepthAttachmentReference(uint32_t index, vk::ImageLayout layout);

		void setSampleCount(vk::SampleCountFlagBits sampleCount);
		vk::SampleCountFlagBits getSampleCount() const;


		vk::SubpassDescription getDescription();

		std::vector<vk::AttachmentReference> getColorAttachments() const;
		std::vector<vk::AttachmentReference> getInputAttachments() const;
		std::vector<vk::AttachmentReference> getResolveAttachments() const;
		

	};

	class RenderProgram {
	private:
		VulkanCore* m_pCore;
		vk::RenderPass m_renderPass;


		std::vector<Subpass> m_subpasses;
		
		std::vector<vk::SubpassDependency> m_dependencies;
		std::vector<vk::AttachmentDescription> m_attachments;

		std::vector<vk::ClearValue> m_clearValues;

	public:
		RenderProgram() = default;

		Subpass& newSubpass();
		void addAttachment(
			vk::ImageLayout intialLayout, 
			vk::ImageLayout finalLayout, 
			vk::Format format, 
			vk::AttachmentLoadOp loadOp, 
			vk::AttachmentStoreOp storeOp, 
			vk::SampleCountFlagBits sampleCount,
			vk::AttachmentLoadOp stencilLoadOp = vk::AttachmentLoadOp::eDontCare, 
			vk::AttachmentStoreOp stencilStoreOp = vk::AttachmentStoreOp::eDontCare);
		vk::AttachmentDescription getAttachment(uint32_t index) const;

		void setClearColor(uint32_t attachmentIndex, Color color);
		void setClearColor(Color color);

		void addSubpassDependency(vk::SubpassDependency dependency);

		ResourceID createPipeline(const ShaderSet& shaders, uint32_t subpassIndex, Extent extent);

		void create(VulkanCore* pCore);
		void destroy();


		void begin(CommandBufferSet* cmd, const FramebufferSet* framebuffer, vk::SubpassContents contents, Rect renderArea);
		void end(CommandBufferSet* cmd);

		vk::RenderPass getRenderPass() const;
	};
}