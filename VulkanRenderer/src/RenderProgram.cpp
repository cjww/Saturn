#include "pch.h"
#include "Resources/RenderProgram.hpp"

namespace sa {
	RenderProgram::RenderProgram()
	{

	}


	Subpass& RenderProgram::newSubpass() {
		return m_subpasses.emplace_back();
	}

	void RenderProgram::addAttachment(vk::ImageLayout intialLayout, vk::ImageLayout finalLayout, vk::Format format, vk::AttachmentLoadOp loadOp, vk::AttachmentStoreOp storeOp, vk::AttachmentLoadOp stencilLoadOp, vk::AttachmentStoreOp stencilStoreOp) {
		m_attachments.push_back(vk::AttachmentDescription{
			.format = format,
			.loadOp = loadOp,
			.storeOp = storeOp,
			.stencilLoadOp = stencilLoadOp,
			.stencilStoreOp = stencilStoreOp,
			.initialLayout = intialLayout,
			.finalLayout = finalLayout,
		});
	}

	void RenderProgram::addSubpassDependency(vk::SubpassDependency dependency) {
		m_dependencies.push_back(dependency);
	}

	void RenderProgram::createPipeline(const ShaderSet& shaders) {
	
		
	}

	void RenderProgram::create(VulkanCore* pCore) {
		m_device = pCore->getDevice();

		std::vector<vk::SubpassDescription> descriptions;
		for (auto& subpass : m_subpasses) {
			descriptions.push_back(subpass.getDescription());
		}

		vk::SubpassDependency dependency{
			.srcSubpass = 0,
			.dstSubpass = 1,
			.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
			.dstStageMask = vk::PipelineStageFlagBits::eFragmentShader,
			.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite,
			.dstAccessMask = vk::AccessFlagBits::eShaderRead,
		};


		vk::RenderPassCreateInfo info;
		
		info.setAttachments(m_attachments)
			.setSubpasses(descriptions)
			.setDependencies(m_dependencies)
			;

		m_renderPass = m_device.createRenderPass(info);

		/*
		vk::FramebufferCreateInfo framebufferInfo;
		framebufferInfo
			.setRenderPass(m_renderPass)
			.;
		//device.createFramebuffer()
		*/


	}

	void RenderProgram::destroy() {
		m_device.destroyRenderPass(m_renderPass);
	}

	void RenderProgram::begin(CommandBufferSet* cmd, FramebufferSet* framebuffer, Color clearColor, Rect renderArea) {
		if (renderArea.extent.width == 0 || renderArea.extent.height == 0) {
			renderArea.extent = framebuffer->getExtent();
		}
		vk::Rect2D rect{
			.offset = { renderArea.offset.x, renderArea.offset.y },
			.extent = { renderArea.extent.width, renderArea.extent.height },
		};

		std::vector<vk::ClearValue> clearValues;

		// TODO add appropriate amount of clear values
		vk::ClearValue value(vk::ClearColorValue{ std::array<float, 4> { clearColor.r, clearColor.g, clearColor.b, clearColor.a }});

		clearValues.push_back(value);

		vk::RenderPassBeginInfo info{
			.renderPass = m_renderPass,
			.framebuffer = framebuffer->getBuffer(cmd->getBufferIndex()),
			.renderArea = rect,
		};

		info.setClearValues(clearValues);

		cmd->getBuffer().beginRenderPass(info, vk::SubpassContents::eInline);
	}

	void RenderProgram::end(CommandBufferSet* cmd) {
		cmd->getBuffer().endRenderPass();
	}

	vk::RenderPass RenderProgram::getRenderPass() const {
		return m_renderPass;
	}

	void Subpass::addAttachmentReference(uint32_t index, vk::ImageLayout layout) {
		
		switch(layout) {
		case vk::ImageLayout::eColorAttachmentOptimal:
			m_colorAttachmentReferences.emplace_back(index, layout);
			break;
		case vk::ImageLayout::eDepthAttachmentOptimal:
		case vk::ImageLayout::eDepthStencilAttachmentOptimal:
			m_depthAttachmentReference = {
				.attachment = index, 
				.layout = layout 
			};
			break;
		case vk::ImageLayout::eShaderReadOnlyOptimal:
			m_inputAttachmentReferences.emplace_back(index, layout);
			break;
		case vk::ImageLayout::ePresentSrcKHR:
			m_resolveAttachmentReferences.emplace_back(index, layout);
			break;
		default:
			throw std::runtime_error("Unsupported attachment format");
			break;
		}
	}
	
	vk::SubpassDescription Subpass::getDescription() {
		return m_description
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
			.setColorAttachments(m_colorAttachmentReferences)
			.setInputAttachments(m_inputAttachmentReferences)
			.setResolveAttachments(m_resolveAttachmentReferences)
			.setPDepthStencilAttachment(m_depthAttachmentReference.has_value() ? &m_depthAttachmentReference.value() : nullptr)
		;
	}
}