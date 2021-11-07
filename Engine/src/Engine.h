#pragma once
#include "IRenderTechnique.h"
#include "ForwardRenderer.h"

#include "ResourceManager.h"
#include "rapidxml\rapidxml.hpp"
#include "Scene.h"

namespace sa {
	class Engine {
	private:

		IRenderTechnique* m_pRenderTechnique;

		std::unordered_map<size_t, Scene> m_scenes;
		Scene* m_currentScene;

		struct FrameTime {
			std::chrono::high_resolution_clock::time_point start;
			std::chrono::duration<double, std::milli> cpu;
			std::chrono::duration<double, std::milli> gpu;
		} m_frameTime;


		void loadXML(const std::filesystem::path& path, rapidxml::xml_document<>& xml, std::string& xmlStr);
		void loadFromFile(const std::filesystem::path& configPath);
	
	public:
		//Engine();
		void setup(RenderWindow* pWindow, const std::filesystem::path& configPath);
		void update(float dt);

		void recordImGui();
		void draw();

		void cleanup();

		std::chrono::duration<double, std::milli> getCPUFrameTime() const;

		IRenderTechnique* getRenderTechnique() const;

		Scene& getScene(const std::string& name);
		Scene* getCurrentScene() const;
		void setScene(const std::string& name);
		void setScene(Scene& scene);




	};
}