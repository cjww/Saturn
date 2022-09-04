#pragma once

#include "Graphics/IRenderTechnique.h"

#include "AssetManager.h"
#include "Scene.h"

#include "Tools/Vector.h"

#include "Tools\utils.h"
#include "ScriptManager.h"

#include <RenderWindow.hpp>

#include <rapidxml\rapidxml.hpp>



namespace sa {

	class Engine {
	private:
		
		IRenderTechnique* m_pRenderTechnique;

		std::unordered_map<size_t, Scene> m_scenes;
		Scene* m_currentScene;

		ScriptManager m_scriptManager;

		struct FrameTime {
			std::chrono::high_resolution_clock::time_point start;
			std::chrono::duration<double, std::milli> cpu;
			std::chrono::duration<double, std::milli> gpu;
		} m_frameTime;

		bool m_isSetup = false;

		void loadXML(const std::filesystem::path& path, rapidxml::xml_document<>& xml, std::string& xmlStr);
		void loadFromFile(const std::filesystem::path& configPath);

		void registerComponents();
	
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

		IRenderTechnique* getRenderTechnique() const;

		Scene& getScene(const std::string& name);
		Scene* getCurrentScene() const;
		void setScene(const std::string& name);
		void setScene(Scene& scene);

		void createSystemScript(const std::string& name);

	};
}