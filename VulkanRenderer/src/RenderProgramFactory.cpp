#include "pch.h"
#include "RenderProgramFactory.hpp"

#include "Resources/RenderProgram.hpp"
#include "Resources/Swapchain.hpp"

#include "Renderer.hpp"

namespace sa {
	
	void RenderProgramFactory::onSubpassEnd(SubpassFactory subpassFactory) {


		if (m_subpasses.empty()) {
			m_subpasses.push_back(subpassFactory);
			return;
		}

		SubpassFactory& previousSubpass = m_subpasses.back();

		std::set<uint32_t> indices;
		std::unordered_map<uint32_t, SubpassAttachmentUsage> usage;
		for (auto& attachment : previousSubpass.getAttachmentReferences()) {
			indices.insert(attachment.index);
			usage[attachment.index] = attachment.usage;
		}

		vk::SubpassDependency dependency;
		bool needDependency = false;
		for (auto& attachment : subpassFactory.getAttachmentReferences()) {
			auto [it, success] = indices.insert(attachment.index);
			if (!success) {
				needDependency = true;
				SubpassAttachmentUsage firstUsage = usage[attachment.index];
				SubpassAttachmentUsage thisUsage = attachment.usage;

				dependency.setSrcSubpass(m_subpasses.size() - 1);
				dependency.setDstSubpass(m_subpasses.size());

				switch (firstUsage) {
				case SubpassAttachmentUsage::DepthTarget:
					dependency.setSrcAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentWrite);
					dependency.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
					break;
				case SubpassAttachmentUsage::ColorTarget:
					dependency.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
					dependency.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
					break;
				case SubpassAttachmentUsage::Input:
					dependency.setSrcAccessMask(vk::AccessFlagBits::eShaderRead);
					dependency.setSrcStageMask(vk::PipelineStageFlagBits::eFragmentShader);
					break;
				default:
					break;
				}

				switch (thisUsage) {
				case SubpassAttachmentUsage::DepthTarget:
					dependency.setDstAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentWrite);
					dependency.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
					break;
				case SubpassAttachmentUsage::ColorTarget:
					dependency.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
					dependency.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
					break;
				case SubpassAttachmentUsage::Input:
					dependency.setDstAccessMask(vk::AccessFlagBits::eShaderRead);
					dependency.setDstStageMask(vk::PipelineStageFlagBits::eFragmentShader);
					break;
				default:
					break;
				}

			}
		}
		if (needDependency) {
			m_pProgram->addSubpassDependency(dependency);
		}

		m_subpasses.push_back(subpassFactory);
	}

	RenderProgramFactory::RenderProgramFactory(VulkanCore* pCore) {
		m_pCore = pCore;
		m_id = sa::ResourceManager::get().insert<RenderProgram>();
		m_pProgram = sa::ResourceManager::get().get<RenderProgram>(m_id);
	}

	RenderProgramFactory& RenderProgramFactory::addColorAttachment() {
		m_pProgram->addAttachment(
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eColorAttachmentOptimal,
			m_pCore->getDefaultColorFormat(),
			vk::AttachmentLoadOp::eDontCare,
			vk::AttachmentStoreOp::eDontCare
		);
		return *this;
	}

	RenderProgramFactory& RenderProgramFactory::addSwapchainAttachment(ResourceID swapchain) {
		Swapchain* pSwapChain = Renderer::get().getSwapchain(swapchain);
		m_pProgram->addAttachment(
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::ePresentSrcKHR,
			pSwapChain->getFormat(),
			vk::AttachmentLoadOp::eClear,
			vk::AttachmentStoreOp::eStore
		);
		return *this;
	}

	RenderProgramFactory& RenderProgramFactory::addDepthAttachment() {
		m_pProgram->addAttachment(
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eDepthStencilAttachmentOptimal,
			m_pCore->getDefaultDepthFormat(),
			vk::AttachmentLoadOp::eClear,
			vk::AttachmentStoreOp::eDontCare
		);
		return *this;
	}

	RenderProgramFactory::SubpassFactory RenderProgramFactory::beginSubpass() {
		return SubpassFactory(this, &m_pProgram->newSubpass());
	}

	RenderProgramFactory& RenderProgramFactory::addSubpassDependency() {

		return *this;
	}

	ResourceID RenderProgramFactory::end() {
		m_pProgram->create(m_pCore);
		return m_id;
	}



	RenderProgramFactory::SubpassFactory::SubpassFactory(RenderProgramFactory* pProgramFactory, Subpass* pSubpass)
		: m_pProgramFactory(pProgramFactory)
		, m_pSubpass(pSubpass)
	{
	}

	RenderProgramFactory::SubpassFactory& RenderProgramFactory::SubpassFactory::addAttachmentReference(uint32_t index, SubpassAttachmentUsage usage) {
		vk::ImageLayout layout;
		switch (usage) {
		case SubpassAttachmentUsage::ColorTarget:
			layout = vk::ImageLayout::eColorAttachmentOptimal;
			break;
		case SubpassAttachmentUsage::DepthTarget:
			layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
			break;
		case SubpassAttachmentUsage::Input:
			layout = vk::ImageLayout::eShaderReadOnlyOptimal;
			break;
		default:
			throw std::runtime_error("Unsupported usage");
			break;
		}

		m_pSubpass->addAttachmentReference(index, layout);

		m_attachmentReferences.emplace_back(index, usage);
		return *this;
	}

	RenderProgramFactory& RenderProgramFactory::SubpassFactory::endSubpass() {
		m_pProgramFactory->onSubpassEnd(*this);
		return *m_pProgramFactory;
	}
	std::vector<RenderProgramFactory::AttachmentReference> RenderProgramFactory::SubpassFactory::getAttachmentReferences() const {
		return m_attachmentReferences;
	}
}
