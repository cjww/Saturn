#include "pch.h"
#include "Resources/Texture.hpp"

#include "VulkanCore.hpp"
#include "Renderer.hpp"
namespace sa {
	Texture2D::Texture2D(VulkanCore* pCore, TextureType type, Extent extent)
		: m_pCore(pCore)
		, m_pImage(nullptr)
		, m_pStagingBuffer(nullptr)
	{
		create(type, extent);
	}

	Texture2D::Texture2D(VulkanCore* pCore, const Image& image)
		: m_pCore(pCore)
		, m_type(TextureType::SAMPLED_COLOR)
		, m_pImage(nullptr)
		, m_pStagingBuffer(nullptr)
	{
		create(m_type, image.getExtent());
		
		m_pStagingBuffer = m_pCore->createBuffer(
			vk::BufferUsageFlagBits::eTransferSrc,
			VMA_MEMORY_USAGE_AUTO,
			VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
			image.getWidth() * image.getHeight() * image.getChannelCount(),
			image.getPixels());
			// transfer data
		DataTransfer transfer {
			.type = DataTransfer::Type::BUFFER_TO_IMAGE,
				.srcBuffer = m_pStagingBuffer,
				.dstImage = m_pImage,
		};
		Renderer::get().queueTransfer(transfer);
	}
	
	void Texture2D::create(TextureType type, Extent extent) {
		switch(type) {
		case sa::TextureType::SAMPLED_COLOR:
			m_pImage = m_pCore->createColorImage2D(
				extent,
				m_pCore->getDefaultColorFormat(),
				vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
				vk::SampleCountFlagBits::e1,
				1,
				1
			);
			
			m_pView = std::make_shared<vk::ImageView>(m_pCore->createImageView(
				vk::ImageViewType::e2D,
				m_pImage->image,
				m_pCore->getDefaultColorFormat(),
				vk::ImageAspectFlagBits::eColor,
				0,
				0
			));

			break;
		case sa::TextureType::ATTACHMENT_COLOR:
			m_pImage = m_pCore->createColorImage2D(
				extent,
				m_pCore->getDefaultColorFormat(),
				vk::ImageUsageFlagBits::eColorAttachment,
				vk::SampleCountFlagBits::e1,
				1,
				1
			);

		case sa::TextureType::ATTACHMENT_DEPTH:
			throw std::runtime_error("unimplemented");
			break;
		default:
			throw std::runtime_error("Invalid texture type");
			break;
		}
	}

	void Texture2D::destroy() {
		m_pCore->getDevice().waitIdle();
		if (m_pStagingBuffer)
			m_pCore->destroyBuffer(m_pStagingBuffer);
		m_pCore->getDevice().destroyImageView(*m_pView);
		m_pCore->destroyImage(m_pImage);
		m_pView = nullptr;
	}

	Extent Texture2D::getExtent() const {
		return { m_pImage->extent.width, m_pImage->extent.height };
	}

	vk::ImageView* Texture2D::getView() const {
		return m_pView.get();
	}
}