#pragma once

#include "Graphics/RenderPipeline.h"
#include "Graphics/RenderTechniques/ForwardPlus.h"
#include "Graphics/RenderLayers/ImGuiRenderLayer.h"
#include "Graphics/RenderLayers/MainRenderLayer.h"

#include "AssetManager.h"
#include "Scene.h"

#include "Tools/Vector.h"

#include "Tools\utils.h"
#include "ScriptManager.h"

#include <RenderWindow.hpp>


namespace sa {


	class Engine : public entt::emitter<Engine> {
	private:
		
		RenderPipeline m_renderPipeline;
		
		Extent m_windowExtent;
		RenderWindow* m_pWindow;

		std::unordered_map<std::string, Scene> m_scenes;
		Scene* m_currentScene;

		struct FrameTime {
			std::chrono::high_resolution_clock::time_point start;
			std::chrono::duration<double, std::milli> cpu;
			std::chrono::duration<double, std::milli> gpu;
		} m_frameTime;

		template<typename T>
		void registerComponentCallBack(Scene& scene);
		void registerComponentCallBacks(Scene& scene);

		void registerMath();

		void reg();
		void onWindowResize(Extent newExtent);

	public:
		// Call this to set up engine
		void setup(sa::RenderWindow* pWindow = nullptr, bool enableImgui = false);

		void cleanup();

		void recordImGui();
		void draw();

		std::chrono::duration<double, std::milli> getCPUFrameTime() const;

		const RenderPipeline& getRenderPipeline() const;

		Scene& getScene(const std::string& name);
		Scene& loadSceneFromFile(const std::filesystem::path& sceneFile);
		void storeSceneToFile(Scene* pScene, const std::filesystem::path& path);

		Scene* getCurrentScene() const;
		Scene* getCurrentScene();

		void setScene(const std::string& name);
		void setScene(Scene& scene);

		std::unordered_map<std::string, Scene>& getScenes();
		void destroyScene(const std::string& name);
		void destroyScenes();

	};
	template<typename T>
	inline void Engine::registerComponentCallBack(Scene& scene) {
		scene.on_construct<T>().connect<&Scene::onComponentConstruct<T>>(scene);
		scene.on_update<T>().connect<&Scene::onComponentUpdate<T>>(scene);
		scene.on_destroy<T>().connect<&Scene::onComponentDestroy<T>>(scene);
	}
}