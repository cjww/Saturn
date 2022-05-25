#include "pch.h"
#include "Resources/RenderProgram.hpp"

namespace sa {

	Subpass& RenderProgram::newSubpass() {
		return m_subpasses.emplace_back();
	}

	void RenderProgram::addAttachment(vk::ImageLayout intialLayout, vk::ImageLayout finalLayout, vk::Format format, vk::AttachmentLoadOp loadOp, vk::AttachmentStoreOp storeOp, vk::AttachmentLoadOp stencilLoadOp, vk::AttachmentStoreOp stencilStoreOp) {
		vk::AttachmentDescription& desc = m_attachments.emplace_back(vk::AttachmentDescription{
			.format = format,
			.loadOp = loadOp,
			.storeOp = storeOp,
			.stencilLoadOp = stencilLoadOp,
			.stencilStoreOp = stencilStoreOp,
			.initialLayout = intialLayout,
			.finalLayout = finalLayout,
		});

		
		if (desc.loadOp == vk::AttachmentLoadOp::eClear) {
			vk::ClearValue value;
			if (VulkanCore::isDepthFormat(desc.format)) {
				value.depthStencil.depth = 1.0f;
				value.depthStencil.stencil = 0;
			}
			else {
				value.color = std::array<float, 4>{0, 0, 0, 0};
			}
			m_clearValues.push_back(value);
		}
	}

	vk::AttachmentDescription RenderProgram::getAttachment(uint32_t index) const {
		return m_attachments.at(index);
	}

	void RenderProgram::setClearColor(uint32_t attachmentIndex, Color color) {
		if (m_attachments.at(attachmentIndex).loadOp != vk::AttachmentLoadOp::eClear)
			throw std::runtime_error((std::string)("Attachment " + attachmentIndex) + " is not clearable");
		m_clearValues.at(attachmentIndex).color = std::array<float, 4> { color.r, color.g, color.b, color.a };
	}

	void RenderProgram::setClearColor(Color color) {
		size_t clearIndex = 0;
		for (size_t i = 0; i < m_attachments.size(); i++) {
			if (m_attachments[i].loadOp == vk::AttachmentLoadOp::eClear) {
				m_clearValues[clearIndex].color = std::array<float, 4>{ color.r, color.g, color.b, color.a };
				clearIndex++;
			}
		}
	}

	void RenderProgram::addSubpassDependency(vk::SubpassDependency dependency) {
		m_dependencies.push_back(dependency);
	}

	ResourceID RenderProgram::createPipeline(const ShaderSet& shaders, uint32_t subpassIndex, Extent extent) {
		PipelineConfig config = {};
		config.colorBlends.resize(m_subpasses.at(subpassIndex).getColorAttachments().size());
		return ResourceManager::get().insert<Pipeline>(m_pCore, m_renderPass, shaders, subpassIndex, extent, config);
	}

	void RenderProgram::create(VulkanCore* pCore) {
		m_pCore = pCore;

		std::vector<vk::SubpassDescription> descriptions;
		for (auto& subpass : m_subpasses) {
			descriptions.push_back(subpass.getDescription());
		}

		vk::RenderPassCreateInfo info;
		info.setAttachments(m_attachments)
			.setSubpasses(descriptions)
			.setDependencies(m_dependencies)
			;

		m_renderPass = m_pCore->getDevice().createRenderPass(info);

	}

	void RenderProgram::destroy() {
		m_pCore->getDevice().destroyRenderPass(m_renderPass);
	}

	void RenderProgram::begin(CommandBufferSet* cmd, FramebufferSet* framebuffer, Rect renderArea) {
		
		if (renderArea.extent.width == 0 || renderArea.extent.height == 0) {
			renderArea.extent = framebuffer->getExtent();
		}
		vk::Rect2D rect{
			.offset = { renderArea.offset.x, renderArea.offset.y },
			.extent = { renderArea.extent.width, renderArea.extent.height },
		};

		vk::RenderPassBeginInfo info{
			.renderPass = m_renderPass,
			.framebuffer = framebuffer->getBuffer(cmd->getBufferIndex()),
			.renderArea = rect,
		};

		info.setClearValues(m_clearValues);

		cmd->getBuffer().beginRenderPass(info, vk::SubpassContents::eInline);
	}

	void RenderProgram::end(CommandBufferSet* cmd) {
		cmd->getBuffer().endRenderPass();
	}

	vk::RenderPass RenderProgram::getRenderPass() const {
		return m_renderPass;
	}
	
	void Subpass::addColorAttachmentReference(uint32_t index, vk::ImageLayout layout) {
		m_colorAttachmentReferences.emplace_back(index, layout);
	}

	void Subpass::addInputAttachmentReference(uint32_t index, vk::ImageLayout layout) {
		m_inputAttachmentReferences.emplace_back(index, layout);
	}

	void Subpass::addResolveAttachmentReference(uint32_t index, vk::ImageLayout layout) {
		m_resolveAttachmentReferences.emplace_back(index, layout);

	}

	void Subpass::setDepthAttachmentReference(uint32_t index, vk::ImageLayout layout) {
		m_depthAttachmentReference = { index, layout };
	}

	vk::SubpassDescription Subpass::getDescription() {
		return m_description
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
			.setResolveAttachments(m_resolveAttachmentReferences)
			.setColorAttachments(m_colorAttachmentReferences)
			.setInputAttachments(m_inputAttachmentReferences)
			.setPDepthStencilAttachment(m_depthAttachmentReference.has_value() ? &m_depthAttachmentReference.value() : nullptr)
		;
	}
	std::vector<vk::AttachmentReference> Subpass::getColorAttachments() const {
		return m_colorAttachmentReferences;
	}

	std::vector<vk::AttachmentReference> Subpass::getInputAttachments() const {
		return m_inputAttachmentReferences;
	}

	std::vector<vk::AttachmentReference> Subpass::getResolveAttachments() const {
		return m_resolveAttachmentReferences;
	}
}