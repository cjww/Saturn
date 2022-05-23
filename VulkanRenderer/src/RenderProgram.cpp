#include "pch.h"
#include "Resources/RenderProgram.hpp"

namespace sa {
	RenderProgram::RenderProgram() {

	}

	Subpass& RenderProgram::beginSubpass() {
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

	void RenderProgram::create(VulkanCore* pCore) {

		addAttachment(vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR, vk::Format::eR16G16B16A16Unorm, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore);
		

		std::vector<vk::SubpassDescription> descriptions;
		descriptions.push_back(beginSubpass()
			.addAttachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal)
			.end());


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
			//.setDependencies(dependency)
			;

		m_renderPass = pCore->getDevice().createRenderPass(info);


		/*
		vk::FramebufferCreateInfo framebufferInfo;
		framebufferInfo
			.setRenderPass(m_renderPass)
			.;
		//device.createFramebuffer()
		*/


	}

	vk::RenderPass RenderProgram::getRenderPass() const { 
		return m_renderPass;
	}

	Subpass& Subpass::addAttachmentReference(uint32_t index, vk::ImageLayout layout) {
		
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
		
		return *this;
	}
	vk::SubpassDescription Subpass::end() {
		return m_description
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
			.setColorAttachments(m_colorAttachmentReferences)
			.setInputAttachments(m_inputAttachmentReferences)
			.setResolveAttachments(m_resolveAttachmentReferences)
			.setPDepthStencilAttachment(m_depthAttachmentReference.has_value() ? &m_depthAttachmentReference.value() : nullptr)
		;
	}
}