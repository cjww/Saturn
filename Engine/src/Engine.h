#pragma once

#include "Graphics/RenderPipeline.h"
#include "Graphics/RenderTechniques/ForwardPlus.h"
#include "Graphics/RenderLayers/ImGuiRenderLayer.h"
#include "Graphics/RenderLayers/MainRenderLayer.h"
#include "Graphics/RenderLayers/BloomRenderLayer.h"

#include "AssetManager.h"
#include "Scene.h"

#include "Tools/Vector.h"

#include "Tools\utils.h"
#include "ScriptManager.h"

#include "Tools/Profiler.h"

#include <RenderWindow.hpp>

namespace sa {


	class Engine : public entt::emitter<Engine> {
	private:
		static std::filesystem::path s_shaderDirectory;
		
		RenderPipeline m_renderPipeline;

		Extent m_windowExtent;
		RenderWindow* m_pWindow;

		Scene* m_currentScene;

		struct FrameTime {
			std::chrono::high_resolution_clock::time_point start;
			std::chrono::duration<double, std::milli> cpu;
			std::chrono::duration<double, std::milli> gpu;
		} m_frameTime;

		
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

		std::chrono::duration<double, std::milli> getCPUFrameTime() const;

		const RenderPipeline& getRenderPipeline() const;
		/*
		Scene& loadSceneFromFile(const std::filesystem::path& sceneFile);
		void storeSceneToFile(Scene* pScene, const std::filesystem::path& path);
		*/

		Scene* getCurrentScene() const;

		void setScene(Scene* scene);
	};
	
}