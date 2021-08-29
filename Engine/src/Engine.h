#pragma once
#include "IRenderTechnique.h"
#include "ForwardRenderer.h"

#include "ResourceManager.h"
#include "rapidxml\rapidxml.hpp"

namespace sa {
	class Engine {
	private:

		IRenderTechnique* m_pRenderTechnique;

		std::vector<Camera*> m_cameras;

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
		void update();

		void recordImGui();
		void draw();

		void cleanup();

		std::chrono::duration<double, std::milli> getCPUFrameTime() const;

		Camera* newCamera();
		Camera* newCamera(const RenderWindow* pWindow);

		void addActiveCamera(Camera* camera);
		void removeActiveCamera(Camera* camera);

		IRenderTechnique* getRenderTechnique() const;

	};
}