#pragma once

#include "Engine.h"
#include "RenderWindow.hpp"

int main(int argc, char** argv);

namespace sa {
	class Appliction;

	class IApplicationLayer {
	private:
		friend class Application;
		Application* m_pAppInstance;
	public:
		IApplicationLayer() = default;
		virtual ~IApplicationLayer() = default;

		virtual void onAttach(Engine& engine, RenderWindow& window) {}
		virtual void onUpdate(float dt) {}
		virtual void onImGuiRender() {}
		virtual void onDetach() {}
	
		Application* getApp() const { return m_pAppInstance; }

	};

	class Application {
	private:
		std::vector<IApplicationLayer*> m_layers;
		unsigned int m_lastLayerIndex;

		bool m_imGuiEnabled;

		sa::Engine m_engine;
		std::unique_ptr<sa::RenderWindow> m_pWindow;
	
		friend int ::main(int argc, char** argv);
		

	public:
		Application(bool enableImGui);
		virtual ~Application();

		void pushLayer(IApplicationLayer* layer);
		void pushOverlay(IApplicationLayer* overlay);


		void run();

	};
	
	static Application* createApplication();


}

