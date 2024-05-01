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

	RenderTarget::RenderTarget(const AssetHeader& header, bool isCompiled)
		: Asset(header, isCompiled)
		, m_renderer(Renderer::Get())
		, m_isActive(true)
		, m_extent({ 256, 256 })
		, m_pOutputTexture(nullptr)
		, m_pDispatcher(nullptr)
		, m_lastTransition(Transition::NONE)
	{

	}

	RenderTarget::RenderTarget() 
		: RenderTarget(AssetHeader{}, false)
	{
	}


	RenderTarget::~RenderTarget() {
		destroy();
	}

	void RenderTarget::initialize(Engine* pEngine, Extent extent) {
		m_extent = extent;
		m_wasResized = true;
		m_pDispatcher = pEngine;
		m_pOutputTexture = nullptr;
	}
	
	void RenderTarget::initialize(Engine* pEngine, RenderWindow* pWindow) {
		m_windowResizedConnection = pEngine->sink<engine_event::WindowResized>().connect<&RenderTarget::onWindowResized>(this);
		initialize(pEngine, pWindow->getCurrentExtent());
	}

	void RenderTarget::destroy() {
		m_windowResizedConnection.release();
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
		m_lastTransition = Transition::NONE;
	}

	bool RenderTarget::wasResized() const {
		return m_wasResized;
	}

	bool RenderTarget::isSampleReady() const {
		return
			m_pOutputTexture != nullptr &&
			m_lastTransition == Transition::FRAGMENT_SHADER_READ;
	}

	void RenderTarget::sync(const RenderContext& context) {
		if (!m_pOutputTexture)
			return;
		m_pOutputTexture->sync(context);
	}


	void RenderTarget::makeSampleReady(const RenderContext& context) {
		if (!m_pOutputTexture)
			return;
		context.barrier(m_pOutputTexture->getTexture(), m_lastTransition, Transition::FRAGMENT_SHADER_READ);
		m_lastTransition = Transition::FRAGMENT_SHADER_READ;
	}

	const Extent& RenderTarget::getExtent() const {
		return m_extent;
	}

	const DynamicTexture* RenderTarget::getOutputTextureDynamic() const {
		return m_pOutputTexture;
	}

	const Texture& RenderTarget::getOutputTexture() const {
		if (m_pOutputTexture)
			return m_pOutputTexture->getTexture(m_pOutputTexture->getNextTextureIndex());
		return *sa::AssetManager::Get().loadDefaultBlackTexture();
	}

	void RenderTarget::setOutputTexture(const DynamicTexture& dynamicTexture, Transition lastTransition) {
		m_pOutputTexture = (DynamicTexture*)&dynamicTexture;
		m_lastTransition = lastTransition;
	}

	void RenderTarget::setActive(bool isActive) {
		m_isActive = isActive;
	}

	bool RenderTarget::isActive() const {
		return m_isActive;
	}

	bool RenderTarget::onLoad(JsonObject& metaData, AssetLoadFlags flags) {
		return true;
	}

	bool RenderTarget::onLoadCompiled(ByteStream& dataInStream, AssetLoadFlags flags) {
		return true;
	}
	
	bool RenderTarget::onWrite(AssetWriteFlags flags) {
		return false;
	}

	bool RenderTarget::onCompile(ByteStream& dataOutStream, AssetWriteFlags flags) {
		return false;
	}

	bool RenderTarget::onUnload() {
		return true;
	}

	RenderTarget* RenderTarget::clone(const std::string& name, const std::filesystem::path& assetDir) const {
		return nullptr;
	}
}
