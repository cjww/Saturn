#include "pch.h"
#include "Application.h"

#include "Tools\Clock.h"

namespace sa {
	void Application::pushLayer(IApplicationLayer* layer) {
		m_layers.insert(m_layers.begin() + m_lastLayerIndex, layer);
		m_lastLayerIndex++;
	}
	
	void Application::pushOverlay(IApplicationLayer* overlay) {
		m_layers.push_back(overlay);
	}
	
	void Application::run() {
		// TODO read application settings

		m_pWindow = std::make_unique<RenderWindow>(1000, 600, "Application");

		m_engine.setup(m_pWindow.get());


		for (const auto layer : m_layers) {
			layer->onAttach(m_engine, *m_pWindow.get());
		}

		m_engine.init();

		Clock clock;
		while (m_pWindow->isOpen()) {
			m_pWindow->pollEvents();

			float dt = clock.restart();

			// Update engine 
			m_engine.update(dt);
			for (const auto layer : m_layers) {
				layer->onUpdate(dt);
			}

			// Imgui
			m_engine.recordImGui();
			for (const auto layer : m_layers) {
				layer->onImGuiRender();
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
