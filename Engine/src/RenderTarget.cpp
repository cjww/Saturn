#include "pch.h"
#include "Graphics/RenderTarget.h"

#include <excpt.h>

#include "Graphics/IRenderTechnique.h"

#include "Engine.h"
#include "Graphics/RenderTechniques/ForwardPlus.h"

namespace sa {

	
	void RenderTarget::onWindowResized(const engine_event::WindowResized& e) {
		this->resize(e.newExtent);
		m_extent = e.newExtent;
	}

	RenderTarget::RenderTarget(const AssetHeader& header) 
		: Asset(header)
		, m_renderer(Renderer::get())
		, m_isActive(true)
		, m_extent({256, 256})
		, m_pOutputTexture(nullptr)
		, m_pDispatcher(nullptr)
	{

	}

	RenderTarget::RenderTarget() 
		: RenderTarget(AssetHeader{})
	{
	}


	RenderTarget::~RenderTarget() {
		destroy();
	}

	void RenderTarget::initialize(Engine* pEngine, Extent extent) {
		m_extent = extent;
		m_wasResized = true;
		/*
		m_mainRenderData.isInitialized = false; // initialize main data in main render pass
		m_bloomData.isInitialized = false; // initialize bloom data in bloom pass
		*/
		m_pDispatcher = pEngine;
		m_pOutputTexture = nullptr;
	}
	
	void RenderTarget::initialize(Engine* pEngine, RenderWindow* pWindow) {
		m_windowResizedConnection = pEngine->sink<engine_event::WindowResized>().connect<&RenderTarget::onWindowResized>(this);
		initialize(pEngine, pWindow->getCurrentExtent());
	}

	void RenderTarget::destroy() {
		m_windowResizedConnection.release();
		//cleanupMainRenderData();
		//cleanupBloomData();

	}

	void RenderTarget::resize(Extent extent) {
		SA_DEBUG_LOG_INFO("RenderTarget Resized (", extent.width, ", ", extent.height, ") UUID: ", getID());
		m_pDispatcher->trigger<sa::engine_event::RenderTargetResized>(sa::engine_event::RenderTargetResized{
			getID(),
			m_extent,
			extent
		});
		m_extent = extent;
		m_wasResized = true;
		m_pOutputTexture = nullptr;
	}

	bool RenderTarget::wasResized() const {
		return m_wasResized;
	}

	bool RenderTarget::isReady() const {
		return
			m_pOutputTexture != nullptr &&
			m_pOutputTexture->isValid();
	}

	const Extent& RenderTarget::getExtent() const {
		return m_extent;
	}

	const DynamicTexture* RenderTarget::getOutputTextureDynamic() const {
		return m_pOutputTexture;
	}

	const Texture& RenderTarget::getOutputTexture() const {
		if (m_pOutputTexture)
			return m_pOutputTexture->getTexture();
		return *sa::AssetManager::get().loadDefaultBlackTexture();
	}

	void RenderTarget::setOutputTexture(const DynamicTexture& dynamicTexture) {
		m_pOutputTexture = (DynamicTexture*)&dynamicTexture;
	}

	void RenderTarget::setActive(bool isActive) {
		m_isActive = isActive;
	}

	bool RenderTarget::isActive() const {
		return m_isActive;
	}

	bool RenderTarget::onLoad(std::ifstream& file, AssetLoadFlags flags) {
		return true;
	}
	
	bool RenderTarget::onWrite(std::ofstream& file, AssetWriteFlags flags) {
		return true;
	}

	bool RenderTarget::onUnload() {
		return true;
	}
}
