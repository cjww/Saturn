#include "pch.h"
#include "RenderProgramFactory.hpp"

#include "internal/RenderProgram.hpp"
#include "internal/Swapchain.hpp"

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
		m_id = ResourceManager::Get().insert<RenderProgram>();
		m_pProgram = ResourceManager::Get().get<RenderProgram>(m_id);
	}

	RenderProgramFactory& RenderProgramFactory::addColorAttachment(AttachmentFlags flags, uint32_t sampleCount) {
		m_pProgram->addAttachment(
			(flags & AttachmentFlagBits::eLoad) ? vk::ImageLayout::eColorAttachmentOptimal : vk::ImageLayout::eUndefined,
			vk::ImageLayout::eShaderReadOnlyOptimal,
			m_pCore->getDefaultColorFormat(),
			(flags & AttachmentFlagBits::eClear) ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eDontCare,
			(flags & AttachmentFlagBits::eStore) ? vk::AttachmentStoreOp::eStore : vk::AttachmentStoreOp::eDontCare,
			(vk::SampleCountFlagBits)sampleCount
		);
		return *this;
	}

	RenderProgramFactory& RenderProgramFactory::addColorAttachment(AttachmentFlags flags, const Texture& framebufferTexture) {
		const DeviceImage* pDeviceImage = (const DeviceImage*)framebufferTexture;
		vk::ImageLayout finalLayout = vk::ImageLayout::eColorAttachmentOptimal;
		if ((framebufferTexture.getUsageFlags() & TextureUsageFlagBits::INPUT_ATTACHMENT ||
			framebufferTexture.getUsageFlags() & TextureUsageFlagBits::SAMPLED) && 
			flags & AttachmentFlagBits::eSampled) {
			finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		}
		if (framebufferTexture.getUsageFlags() & TextureUsageFlagBits::STORAGE) {
			finalLayout = vk::ImageLayout::eGeneral;
		}
		m_pProgram->addAttachment(
			(flags & AttachmentFlagBits::eLoad) ? vk::ImageLayout::eColorAttachmentOptimal : vk::ImageLayout::eUndefined,
			finalLayout,
			pDeviceImage->format,
			(flags & AttachmentFlagBits::eClear) ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eDontCare,
			(flags & AttachmentFlagBits::eStore) ? vk::AttachmentStoreOp::eStore : vk::AttachmentStoreOp::eDontCare,
			pDeviceImage->sampleCount
		);


		DeviceImage* pImage = (DeviceImage*)framebufferTexture;
		pImage->layout = finalLayout;
		return *this;
	}

	RenderProgramFactory& RenderProgramFactory::addColorAttachment(AttachmentFlags flags, Format format, uint32_t sampleCount) {
		vk::ImageLayout finalLayout = vk::ImageLayout::eColorAttachmentOptimal;
		if (flags & AttachmentFlagBits::eSampled) {
			finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		}
		m_pProgram->addAttachment(
			(flags & AttachmentFlagBits::eLoad) ? vk::ImageLayout::eColorAttachmentOptimal : vk::ImageLayout::eUndefined,
			finalLayout,
			(vk::Format)format,
			(flags & AttachmentFlagBits::eClear) ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eDontCare,
			(flags & AttachmentFlagBits::eStore) ? vk::AttachmentStoreOp::eStore : vk::AttachmentStoreOp::eDontCare,
			(vk::SampleCountFlagBits)sampleCount
		);
		return *this;
	}


	RenderProgramFactory& RenderProgramFactory::addSwapchainAttachment(ResourceID swapchain) {
		Swapchain* pSwapChain = RenderContext::GetSwapchain(swapchain);
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

	RenderProgramFactory& RenderProgramFactory::addDepthAttachment(AttachmentFlags flags, uint32_t sampleCount) {
		m_pProgram->addAttachment(
			((flags & (AttachmentFlagBits::eLoad | AttachmentFlagBits::eSampled)) == (AttachmentFlagBits::eLoad | AttachmentFlagBits::eSampled)) ?
				vk::ImageLayout::eShaderReadOnlyOptimal : (flags & AttachmentFlagBits::eLoad) ? 
					vk::ImageLayout::eDepthStencilAttachmentOptimal : vk::ImageLayout::eUndefined,
			
			((flags & (AttachmentFlagBits::eStore | AttachmentFlagBits::eSampled)) == (AttachmentFlagBits::eStore | AttachmentFlagBits::eSampled)) ?
				vk::ImageLayout::eShaderReadOnlyOptimal : vk::ImageLayout::eDepthStencilAttachmentOptimal,
			
			m_pCore->getDefaultDepthFormat(),
			
			(flags & AttachmentFlagBits::eClear) ? 
				vk::AttachmentLoadOp::eClear : (flags & AttachmentFlagBits::eLoad) ?
					vk::AttachmentLoadOp::eLoad : vk::AttachmentLoadOp::eDontCare,

			(flags & AttachmentFlagBits::eStore) ? vk::AttachmentStoreOp::eStore : vk::AttachmentStoreOp::eDontCare,
			
			(vk::SampleCountFlagBits)sampleCount
		);
		return *this;
	}

	RenderProgramFactory& RenderProgramFactory::addDepthAttachment(AttachmentFlags flags, const Texture& framebufferTexture) {
		const DeviceImage* pDeviceImage = (const DeviceImage*)framebufferTexture;

		vk::ImageLayout finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
		if ((framebufferTexture.getUsageFlags() & TextureUsageFlagBits::INPUT_ATTACHMENT ||
			framebufferTexture.getUsageFlags() & TextureUsageFlagBits::SAMPLED) &&
			flags & AttachmentFlagBits::eSampled) {
			finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		}

		m_pProgram->addAttachment(
			((flags & (AttachmentFlagBits::eLoad | AttachmentFlagBits::eSampled)) == (AttachmentFlagBits::eLoad | AttachmentFlagBits::eSampled)) ?
				vk::ImageLayout::eShaderReadOnlyOptimal : (flags & AttachmentFlagBits::eLoad) ?
					vk::ImageLayout::eDepthStencilAttachmentOptimal : vk::ImageLayout::eUndefined,

			((flags & (AttachmentFlagBits::eStore | AttachmentFlagBits::eSampled)) == (AttachmentFlagBits::eStore | AttachmentFlagBits::eSampled)) ?
				vk::ImageLayout::eShaderReadOnlyOptimal : vk::ImageLayout::eDepthStencilAttachmentOptimal,
			
			pDeviceImage->format,
			(flags & AttachmentFlagBits::eClear) ? vk::AttachmentLoadOp::eClear : (flags & AttachmentFlagBits::eLoad) ? vk::AttachmentLoadOp::eLoad : vk::AttachmentLoadOp::eDontCare,
			(flags & AttachmentFlagBits::eStore) ? vk::AttachmentStoreOp::eStore : vk::AttachmentStoreOp::eDontCare,
			pDeviceImage->sampleCount
		);

		DeviceImage* pImage = (DeviceImage*)framebufferTexture;
		pImage->layout = finalLayout;
		return *this;
	}

	RenderProgramFactory& RenderProgramFactory::addDepthAttachment(AttachmentFlags flags, Format format, uint32_t sampleCount) {
		m_pProgram->addAttachment(
			((flags & (AttachmentFlagBits::eLoad | AttachmentFlagBits::eSampled)) == (AttachmentFlagBits::eLoad | AttachmentFlagBits::eSampled)) ?
				vk::ImageLayout::eShaderReadOnlyOptimal : (flags & AttachmentFlagBits::eLoad) ?
					vk::ImageLayout::eDepthStencilAttachmentOptimal : vk::ImageLayout::eUndefined,

			((flags & (AttachmentFlagBits::eStore | AttachmentFlagBits::eSampled)) == (AttachmentFlagBits::eStore | AttachmentFlagBits::eSampled)) ?
				vk::ImageLayout::eShaderReadOnlyOptimal : vk::ImageLayout::eDepthStencilAttachmentOptimal,
			
			(vk::Format)format,
			(flags & AttachmentFlagBits::eClear) ? vk::AttachmentLoadOp::eClear : (flags & AttachmentFlagBits::eLoad) ? vk::AttachmentLoadOp::eLoad : vk::AttachmentLoadOp::eDontCare,
			(flags & AttachmentFlagBits::eStore) ? vk::AttachmentStoreOp::eStore : vk::AttachmentStoreOp::eDontCare,
			(vk::SampleCountFlagBits)sampleCount
		);
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

	RenderProgramFactory& RenderProgramFactory::addDepthDependency(uint32_t srcSubpass, uint32_t dstSubpass) {
		vk::SubpassDependency dep{
			.srcSubpass = srcSubpass,
			.dstSubpass = dstSubpass,
			.srcStageMask = vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests,
			.dstStageMask = vk::PipelineStageFlagBits::eFragmentShader,
			.srcAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite,
			.dstAccessMask = vk::AccessFlagBits::eShaderRead,
		};
		m_pProgram->addSubpassDependency(dep);
		return *this;
	}

	RenderProgramFactory& RenderProgramFactory::addSwapchainDependency(uint32_t srcSubpass, uint32_t dstSubpass) {
		vk::SubpassDependency dep{
			.srcSubpass = srcSubpass,
			.dstSubpass = dstSubpass,
			.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
			.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
			.srcAccessMask = vk::AccessFlagBits::eNone,
			.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite,
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
