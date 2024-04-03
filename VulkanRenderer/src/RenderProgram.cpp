#include "pch.h"
#include "internal/RenderProgram.hpp"

namespace sa {

	Subpass& RenderProgram::newSubpass() {
		return m_subpasses.emplace_back();
	}

	void RenderProgram::addAttachment(vk::ImageLayout intialLayout, vk::ImageLayout finalLayout, vk::Format format, vk::AttachmentLoadOp loadOp, vk::AttachmentStoreOp storeOp, vk::SampleCountFlagBits sampleCount, vk::AttachmentLoadOp stencilLoadOp, vk::AttachmentStoreOp stencilStoreOp) {
		vk::AttachmentDescription& desc = m_attachments.emplace_back(vk::AttachmentDescription{
			.format = format,
			.samples = sampleCount,
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
		if (index >= m_attachments.size())
			throw std::runtime_error("Invalid attachment index: " + std::to_string(index));
		return m_attachments.at(index);
	}

	void RenderProgram::setClearColor(uint32_t attachmentIndex, Color color) {
		if (m_attachments.at(attachmentIndex).loadOp != vk::AttachmentLoadOp::eClear)
			throw std::runtime_error("Attachment " + std::to_string(attachmentIndex) + " is not clearable");
		m_clearValues.at(attachmentIndex).color = std::array<float, 4> { color.r, color.g, color.b, color.a };
	}

	void RenderProgram::setClearColor(Color color) {
		size_t clearIndex = 0;
		for (size_t i = 0; i < m_attachments.size(); i++) {
			if (m_attachments[i].loadOp == vk::AttachmentLoadOp::eClear) {
				if (!VulkanCore::isDepthFormat(m_attachments[i].format)) {
					m_clearValues[clearIndex].color = std::array<float, 4>{ color.r, color.g, color.b, color.a };
				}
				clearIndex++;
			}
		}
	}

	void RenderProgram::addSubpassDependency(vk::SubpassDependency dependency) {
		m_dependencies.push_back(dependency);
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

	void RenderProgram::begin(CommandBufferSet* cmd, const FramebufferSet* framebuffer, vk::SubpassContents contents, Rect renderArea) {
		
		if (renderArea.extent.width == 0 || renderArea.extent.height == 0) {
			renderArea.extent = framebuffer->getExtent();
		}
		vk::Rect2D rect{
			.offset = { renderArea.offset.x, renderArea.offset.y },
			.extent = { renderArea.extent.width, renderArea.extent.height },
		};

		uint32_t frameBufferIndex = cmd->getBufferIndex() % framebuffer->getBufferCount();
		Swapchain* pSwapchain = framebuffer->getSwapchain();
		if (pSwapchain) {
			frameBufferIndex = pSwapchain->getImageIndex();
		}
		vk::RenderPassBeginInfo info{
			.renderPass = m_renderPass,
			.framebuffer = framebuffer->getBuffer(frameBufferIndex),
			.renderArea = rect,
		};
		
		info.setClearValues(m_clearValues);

		for (size_t i = 0; i < framebuffer->getTextureCount(); i++) {
			const sa::Texture& tex = framebuffer->getTexture(i);
			sa::DeviceImage* pImage = tex;
			pImage->layout = m_attachments.at(i).finalLayout;
		}

		cmd->getBuffer().beginRenderPass(info, contents);
	}

	void RenderProgram::end(CommandBufferSet* cmd) {

		cmd->getBuffer().endRenderPass();
	}

	vk::RenderPass RenderProgram::getRenderPass() const {
		return m_renderPass;
	}

	const Subpass& RenderProgram::getSubpass(uint32_t index) const {
		return m_subpasses.at(index);
	}

	Subpass::Subpass()
		: m_sampleCount(vk::SampleCountFlagBits::e1)
	{

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

	void Subpass::setSampleCount(vk::SampleCountFlagBits sampleCount) {
		m_sampleCount = sampleCount;
	}

	vk::SampleCountFlagBits Subpass::getSampleCount() const {
		return m_sampleCount;
	}

	vk::SubpassDescription Subpass::getDescription() {
		return m_description
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
			.setColorAttachments(m_colorAttachmentReferences)
			.setPResolveAttachments(m_resolveAttachmentReferences.data())
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