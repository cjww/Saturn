#pragma once

#include "Engine.h"
#include "RenderWindow.hpp"

int main(int argc, char** argv);

namespace sa {

	class IApplicationLayer {
	private:

	public:
		IApplicationLayer() = default;
		virtual ~IApplicationLayer() = default;

		virtual void onAttach(Engine& engine, RenderWindow& window) {}
		virtual void onUpdate(float dt) {}
		virtual void onImGuiRender() {}
		virtual void onDetach() {}
	};

	class Application {
	private:
		std::vector<IApplicationLayer*> m_layers;
		unsigned int m_lastLayerIndex;

		bool m_imGuiEnabled;

		sa::Engine m_engine;
		std::unique_ptr<sa::RenderWindow> m_pWindow;
	
		friend int ::main(int argc, char** argv);
		
		inline static Application* m_thisInstance = nullptr;

	public:
		bool simulate;

		Application(bool enableImGui);
		virtual ~Application();

		static Application* get();

		void pushLayer(IApplicationLayer* layer);
		void pushOverlay(IApplicationLayer* overlay);


		void run();

		sa::RenderWindow* getWindow() const;
		const sa::Engine& getEngine() const;


	};
	
	static Application* createApplication();


}

