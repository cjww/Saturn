#include "pch.h"
#include "Application.h"

#include "Tools\Clock.h"
#include "Tools\Profiler.h"

namespace sa {
	Application::Application(bool enableImGui) {
		m_imGuiEnabled = enableImGui;
	}

	void Application::pushLayer(IApplicationLayer* layer) {
		m_layers.insert(m_layers.begin() + m_lastLayerIndex, layer);
		m_lastLayerIndex++;
	}
	
	void Application::pushOverlay(IApplicationLayer* overlay) {
		m_layers.push_back(overlay);
	}
	
	void Application::run() {
		SA_PROFILE_FUNCTION();

		{
			SA_PROFILE_SCOPE("Setup");
			// TODO read application settings
			m_pWindow = std::make_unique<RenderWindow>(1000, 600, "Application");

			m_engine.setup(m_pWindow.get(), m_imGuiEnabled);


			for (const auto layer : m_layers) {
				layer->onAttach(m_engine, *m_pWindow.get());
			}

			m_engine.init();
		}

		Clock clock;
		while (m_pWindow->isOpen()) {
			SA_PROFILE_SCOPE("MainLoop");
			m_pWindow->pollEvents();

			float dt = clock.restart();

			// Update engine 
			m_engine.update(dt);
			for (const auto layer : m_layers) {
				layer->onUpdate(dt);
			}

			// Imgui
			if (m_imGuiEnabled) {
				m_engine.recordImGui();
				for (const auto layer : m_layers) {
					layer->onImGuiRender();
				}
			}

			// Draw engine
			m_engine.draw();

		}


		for (const auto layer : m_layers) {
			layer->onDetach();
		}

		m_pWindow->close();
		m_engine.cleanup();

	}
}
