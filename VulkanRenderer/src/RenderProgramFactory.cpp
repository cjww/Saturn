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

		for (auto& attachment : subpassFactory.getAttachmentReferences()) {
			auto [it, success] = indices.insert(attachment.index);
			if (!success) {
				vk::SubpassDependency dependency;
				SubpassAttachmentUsage firstUsage = usage[attachment.index];
				SubpassAttachmentUsage thisUsage = attachment.usage;

				dependency.setSrcSubpass(m_subpasses.size() - 1);
				dependency.setDstSubpass(m_subpasses.size());

				switch (firstUsage) {
				case SubpassAttachmentUsage::DepthTarget:
					dependency.setSrcAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentWrite);
					dependency.setSrcStageMask(vk::PipelineStageFlagBits::eEarlyFragmentTests);
					break;
				case SubpassAttachmentUsage::Resolve:
				case SubpassAttachmentUsage::ColorTarget:
					dependency.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
					dependency.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
					break;
				case SubpassAttachmentUsage::Input:
					dependency.setSrcAccessMask(vk::AccessFlagBits::eInputAttachmentRead);
					dependency.setSrcStageMask(vk::PipelineStageFlagBits::eFragmentShader);
					break;
				default:
					break;
				}

				switch (thisUsage) {
				case SubpassAttachmentUsage::DepthTarget:
					dependency.setDstAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentWrite);
					dependency.setDstStageMask(vk::PipelineStageFlagBits::eEarlyFragmentTests);
					break;
				case SubpassAttachmentUsage::Resolve:
				case SubpassAttachmentUsage::ColorTarget:
					dependency.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
					dependency.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
					break;
				case SubpassAttachmentUsage::Input:
					dependency.setDstAccessMask(vk::AccessFlagBits::eInputAttachmentRead);
					dependency.setDstStageMask(vk::PipelineStageFlagBits::eFragmentShader);
					break;
				default:
					break;
				}

				m_pProgram->addSubpassDependency(dependency);
			}
		}
		
		m_subpasses.push_back(subpassFactory);
	}

	RenderProgramFactory::RenderProgramFactory(VulkanCore* pCore) {
		m_pCore = pCore;
		m_id = sa::ResourceManager::get().insert<RenderProgram>();
		m_pProgram = sa::ResourceManager::get().get<RenderProgram>(m_id);
	}

	RenderProgramFactory& RenderProgramFactory::addColorAttachment(bool store) {
		m_pProgram->addAttachment(
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eShaderReadOnlyOptimal,
			m_pCore->getDefaultColorFormat(),
			vk::AttachmentLoadOp::eClear,
			(store) ? vk::AttachmentStoreOp::eStore : vk::AttachmentStoreOp::eDontCare,
			vk::SampleCountFlagBits::e1
		);
		return *this;
	}

	RenderProgramFactory& RenderProgramFactory::addColorAttachment(bool store, const Texture2D& framebufferTexture) {
		const DeviceImage* pDeviceImage = (const DeviceImage*)framebufferTexture;
		vk::ImageLayout finalLayout = vk::ImageLayout::eColorAttachmentOptimal;
		if (framebufferTexture.getTypeFlags() & TextureTypeFlagBits::INPUT_ATTACHMENT ||
			framebufferTexture.getTypeFlags() & TextureTypeFlagBits::SAMPLED) {
			finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		}
		if (framebufferTexture.getTypeFlags() & TextureTypeFlagBits::STORAGE) {
			finalLayout = vk::ImageLayout::eGeneral;
		}
		m_pProgram->addAttachment(
			vk::ImageLayout::eUndefined,
			finalLayout,
			pDeviceImage->format,
			vk::AttachmentLoadOp::eClear,
			(store)? vk::AttachmentStoreOp::eStore : vk::AttachmentStoreOp::eDontCare,
			pDeviceImage->sampleCount
		);

		DeviceImage* pImage = (DeviceImage*)framebufferTexture;
		pImage->layout = finalLayout;
		return *this;
	}

	RenderProgramFactory& RenderProgramFactory::addSwapchainAttachment(ResourceID swapchain) {
		Swapchain* pSwapChain = RenderContext::getSwapchain(swapchain);
		m_pProgram->addAttachment(
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::ePresentSrcKHR,
			pSwapChain->getFormat(),
			vk::AttachmentLoadOp::eClear,
			vk::AttachmentStoreOp::eStore,
			vk::SampleCountFlagBits::e1
		);
		return *this;
	}

	RenderProgramFactory& RenderProgramFactory::addDepthAttachment() {
		m_pProgram->addAttachment(
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eDepthStencilAttachmentOptimal,
			m_pCore->getDefaultDepthFormat(),
			vk::AttachmentLoadOp::eClear,
			vk::AttachmentStoreOp::eDontCare,
			vk::SampleCountFlagBits::e1
		);
		return *this;
	}

	RenderProgramFactory& RenderProgramFactory::addDepthAttachment(const Texture2D& framebufferTexture, bool store) {
		const DeviceImage* pDeviceImage = (const DeviceImage*)framebufferTexture;

		vk::ImageLayout finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
		if (framebufferTexture.getTypeFlags() & TextureTypeFlagBits::INPUT_ATTACHMENT ||
			framebufferTexture.getTypeFlags() & TextureTypeFlagBits::SAMPLED) {
			finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		}

		m_pProgram->addAttachment(
			vk::ImageLayout::eUndefined,
			finalLayout,
			pDeviceImage->format,
			vk::AttachmentLoadOp::eClear,
			(store) ? vk::AttachmentStoreOp::eStore : vk::AttachmentStoreOp::eDontCare,
			pDeviceImage->sampleCount
		);

		DeviceImage* pImage = (DeviceImage*)framebufferTexture;
		pImage->layout = finalLayout;
		return *this;
	}

	RenderProgramFactory::SubpassFactory RenderProgramFactory::beginSubpass() {
		return SubpassFactory(this, &m_pProgram->newSubpass());
	}

	RenderProgramFactory& RenderProgramFactory::addColorDependency(uint32_t srcSubpass, uint32_t dstSubpass) {
		vk::SubpassDependency dep{
			.srcSubpass = srcSubpass,
			.dstSubpass = dstSubpass,
			.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
			.dstStageMask = vk::PipelineStageFlagBits::eFragmentShader,
			.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite,
			.dstAccessMask = vk::AccessFlagBits::eShaderRead,
		};
		m_pProgram->addSubpassDependency(dep);
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
		vk::AttachmentDescription desc = m_pProgramFactory->m_pProgram->getAttachment(index);
		vk::ImageLayout layout;
		switch (usage) {
		case SubpassAttachmentUsage::ColorTarget:
			m_pSubpass->setSampleCount(desc.samples);
			layout = vk::ImageLayout::eColorAttachmentOptimal;
			m_pSubpass->addColorAttachmentReference(index, layout);
			break;
		case SubpassAttachmentUsage::DepthTarget:
			layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
			m_pSubpass->setDepthAttachmentReference(index, layout);
			break;
		case SubpassAttachmentUsage::Input:
			layout = vk::ImageLayout::eShaderReadOnlyOptimal;
			m_pSubpass->addInputAttachmentReference(index, layout);
			break;
		case SubpassAttachmentUsage::Resolve:
			layout = vk::ImageLayout::eColorAttachmentOptimal;
			m_pSubpass->addResolveAttachmentReference(index, layout);
			break;

		default:
			throw std::runtime_error("Unsupported usage");
			return *this;
		}


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
