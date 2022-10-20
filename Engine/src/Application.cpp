#include "pch.h"
#include "Application.h"

#include "Tools\Clock.h"
#include "Tools\Profiler.h"

namespace sa {
	Application::Application(bool enableImGui) {
		SA_PROFILE_FUNCTION();
		m_imGuiEnabled = enableImGui;
		// TODO read application settings
		m_pWindow = std::make_unique<RenderWindow>(1400, 800, "Application");

		m_engine.setup(m_pWindow.get(), m_imGuiEnabled);

		m_thisInstance = this;
	}

	Application::~Application() {
		for (auto& layer : m_layers) {
			delete layer;
		}
	}

	Application* Application::get() {
		return m_thisInstance;
	}
	
	void Application::pushLayer(IApplicationLayer* layer) {
		m_layers.insert(m_layers.begin() + m_lastLayerIndex, layer);
		m_lastLayerIndex++;
		layer->onAttach(m_engine, *m_pWindow.get());
	}
	
	void Application::pushOverlay(IApplicationLayer* overlay) {
		m_layers.push_back(overlay);
		overlay->onAttach(m_engine, *m_pWindow.get());
	}
	
	void Application::run() {
		SA_PROFILE_FUNCTION();
		
		m_engine.init();

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

	sa::RenderWindow* Application::getWindow() const {
		return m_pWindow.get();
	}

	const sa::Engine& Application::getEngine() const {
		return m_engine;
	}

}
