#pragma once

#include "Graphics/RenderPipeline.h"
#include "Graphics/WindowRenderer.h"

#include "AssetManager.h"
#include "Scene.h"

#include <RenderWindow.hpp>

namespace sa {


	class Engine : public entt::dispatcher {
	private:
		static std::filesystem::path s_shaderDirectory;

		RenderPipeline m_renderPipeline;
		IWindowRenderer* m_pWindowRenderer;

		RenderTarget m_mainRenderTarget;

		Extent m_windowExtent;
		RenderWindow* m_pWindow;

		Scene* m_currentScene;
		
		void registerMath();

		void reg();
		void onWindowResize(Extent newExtent);

	public:
		static const std::filesystem::path& getShaderDirectory();
		static void setShaderDirectory(const std::filesystem::path& path);


		// Call this to set up engine
		void setup(sa::RenderWindow* pWindow = nullptr, bool enableImgui = false);

		void cleanup();

		void recordImGui();
		void draw();

		const RenderPipeline& getRenderPipeline() const;

		const RenderTarget& getMainRenderTarget() const;

		void setWindowRenderer(IWindowRenderer* pWindowRenderer);

		Scene* getCurrentScene() const;

		void setScene(Scene* scene);

		void onSceneSet(engine_event::SceneSet& e);

	};
	
}