#pragma once

#include "Graphics/RenderPipeline.h"

#include "AssetManager.h"
#include "Scene.h"

#include "Tools/Vector.h"

#include "Tools\utils.h"
#include "ScriptManager.h"
#include "PhysicsSystem.h"

#include <RenderWindow.hpp>

#include <rapidxml\rapidxml.hpp>



namespace sa {

	class Engine : public entt::emitter<Engine> {
	private:
		
		RenderPipeline m_renderPipeline;
		bool m_isImGuiRecording;

		Extent m_windowExtent;
		RenderWindow* m_pWindow;

		std::unordered_map<std::string, Scene> m_scenes;
		Scene* m_currentScene;

		struct FrameTime {
			std::chrono::high_resolution_clock::time_point start;
			std::chrono::duration<double, std::milli> cpu;
			std::chrono::duration<double, std::milli> gpu;
		} m_frameTime;

		bool m_isSetup = false;

		void loadXML(const std::filesystem::path& path, rapidxml::xml_document<>& xml, std::string& xmlStr);
		void loadFromFile(const std::filesystem::path& configPath);

		void registerMath();

		void reg();
		void onWindowResize(Extent newExtent);

	public:
		// Call this to set up engine
		void setup(sa::RenderWindow* pWindow = nullptr, bool enableImgui = false);

		// Call this reight before the main loop
		void init();

		void update(float dt);

		void cleanup();

		void recordImGui();
		void draw();

		std::chrono::duration<double, std::milli> getCPUFrameTime() const;

		const RenderPipeline& getRenderPipeline() const;

		Scene& getScene(const std::string& name);
		Scene* getCurrentScene() const;
		void setScene(const std::string& name);
		void setScene(Scene& scene);

		std::unordered_map<std::string, Scene>& getScenes();

	};
}