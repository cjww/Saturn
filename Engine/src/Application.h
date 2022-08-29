#pragma once

#include "Engine.h"
#include "RenderWindow.hpp"

int main(int argc, char** argv);

namespace sa {
	class IApplicationLayer {
	private:
	
	public:
		
		virtual void onAttach(Engine& engine, RenderWindow& window) {}
		virtual void onUpdate(float dt) {}
		virtual void onImGuiRender() {}
		virtual void onDetach() {}
	

	};

	class Application {
	private:
		std::vector<IApplicationLayer*> m_layers;
		unsigned int m_lastLayerIndex;

		sa::Engine m_engine;
		std::unique_ptr<sa::RenderWindow> m_pWindow;
	
		friend int ::main(int argc, char** argv);
	public:
		Application() = default;
		~Application() = default;

		void pushLayer(IApplicationLayer* layer);
		void pushOverlay(IApplicationLayer* overlay);


		void run();

	};
	
	static Application* createApplication();


}

