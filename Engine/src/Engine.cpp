#include "pch.h"
#include "Engine.h"

#include <vulkan/vulkan_core.h>

#include "Graphics/RenderTechniques/ForwardPlus.h"
#include "Graphics/RenderLayers/BloomRenderLayer.h"
#include "Graphics/RenderLayers/ShadowRenderLayer.h"

#include "Lua/Ref.h"
#include "Tools/Vector.h"
#include "Tools/Profiler.h"

#include "Lua\LuaTypes.h"

namespace sa {
	std::filesystem::path Engine::s_shaderDirectory = std::filesystem::current_path();


	void Engine::onWindowResize(Extent newExtent) {
		m_pWindowRenderer->onWindowResize(newExtent);

		trigger<engine_event::WindowResized>(engine_event::WindowResized{ m_windowExtent, newExtent });
		m_windowExtent = newExtent;
	}

	void Engine::onRenderTargetResize(sa::engine_event::RenderTargetResized e) {
		m_renderPipeline.onRenderTargetResize(e.renderTargetID, e.oldExtent, e.newExtent);
	}

	const std::filesystem::path& Engine::getShaderDirectory() {
		return s_shaderDirectory;
	}

	void Engine::setShaderDirectory(const std::filesystem::path& path) {
		s_shaderDirectory = std::filesystem::absolute(path);
	}

	void Engine::setup(sa::RenderWindow* pWindow, bool enableImgui) {
		SA_PROFILE_FUNCTION();
		
		m_currentScene = nullptr;
		m_pWindow = pWindow;
		
		registerAllComponents();

		LuaAccessable::registerType<Engine>();
		LuaAccessable::registerType<Entity>();
		LuaAccessable::registerType<Ref>();
		LuaAccessable::registerType<AssetManager>();

		LuaAccessable::registerType<Vector2>();
		LuaAccessable::registerType<Vector3>();
		LuaAccessable::registerType<Vector4>();
		LuaAccessable::registerType<glm::quat>();

		if (pWindow) {
			
			setWindowRenderer(new WindowRenderer(m_pWindow));

			pWindow->setResizeCallback(std::bind(&Engine::onWindowResize, this, std::placeholders::_1));
			m_windowExtent = pWindow->getCurrentExtent();
			m_mainRenderTarget.initialize(this, m_pWindow);
		}
		sink<engine_event::SceneSet>().connect<&Engine::onSceneSet>(this);
		sink<engine_event::RenderTargetResized>().connect<&Engine::onRenderTargetResize>(this);
	}

	void Engine::setupDefaultRenderPipeline() {
		m_renderPipeline.addLayer(new ShadowRenderLayer);
		m_renderPipeline.addLayer(new ForwardPlus(m_renderPipeline.getLayer<ShadowRenderLayer>()));
		m_renderPipeline.addLayer(new BloomRenderLayer);
	}

	void Engine::cleanup() {
		if (m_pWindowRenderer)
			delete m_pWindowRenderer;
		AssetManager::get().clear();
	}

	void Engine::recordImGui() {
		SA_PROFILE_FUNCTION();
		m_renderPipeline.beginFrameImGUI();
	}

	void Engine::draw() {
		SA_PROFILE_FUNCTION();

		if (!m_pWindow)
			return;

		RenderContext context = m_pWindow->beginFrame();
		if (!context)
			return;
		
		Scene* pCurrentScene = getCurrentScene();
		if (pCurrentScene) {
			pCurrentScene->render(context, m_renderPipeline, m_mainRenderTarget);
		}
		trigger<engine_event::OnRender>(engine_event::OnRender{ &context, &m_renderPipeline });

		if (pCurrentScene)
			pCurrentScene->getDynamicSceneCollection().swap();

		m_pWindowRenderer->render(context, m_mainRenderTarget.getOutputTexture());
		{
			SA_PROFILE_SCOPE("Display");
			m_pWindow->display();
		}
	}

	RenderPipeline& Engine::getRenderPipeline() {
		return m_renderPipeline;
	}

	const RenderTarget& Engine::getMainRenderTarget() const {
		return m_mainRenderTarget;
	}

	void Engine::setWindowRenderer(IWindowRenderer* pWindowRenderer) {
		if (m_pWindowRenderer)
			delete m_pWindowRenderer;
		m_pWindowRenderer = pWindowRenderer;
	}

	Scene* Engine::getCurrentScene() const {
		return m_currentScene.getAsset();
	}

	void Engine::setScene(Scene* scene) {
		SA_PROFILE_FUNCTION();

		if (m_currentScene.getAsset() == scene)
			return;

		if (m_currentScene) 
			m_currentScene.getProgress()->wait();

		if (scene) {
			scene->hold();
			scene->getProgress().wait();
		}

		trigger<engine_event::SceneSet>(engine_event::SceneSet{ m_currentScene.getAsset(), scene });

		m_currentScene = scene;

		if(scene)
			scene->release();
		

	}

	void Engine::onSceneSet(engine_event::SceneSet& e) {
		if (e.oldScene) {
			e.oldScene->onRuntimeStop();
		}
		e.newScene->onRuntimeStart();
	}
}

