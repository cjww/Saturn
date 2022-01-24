#include "Engine.h"

#include "entt/entt.hpp"

namespace sa {
	void Engine::loadXML(const std::filesystem::path& path, rapidxml::xml_document<>& xml, std::string& xmlStr) {
		std::ifstream file(path);
		if (!file.is_open()) {
			throw std::runtime_error("Failed to open file " + path.string());
		}
		xmlStr.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
		xmlStr.push_back('\0');
		file.close();
		xml.parse<0>(xmlStr.data());
	}

	void Engine::loadFromFile(const std::filesystem::path& configPath) {
		using namespace rapidxml;
		xml_document<> doc;
		std::string docStr;
		loadXML(configPath, doc, docStr);
		xml_node<>* root = doc.first_node();
		xml_node<>* rendererNode = root->first_node("Renderer");
		if (rendererNode) {
			xml_attribute<>* api = rendererNode->first_attribute("API", 0, false);

			xml_attribute<>* renderTechnique = rendererNode->first_attribute("RenderTechnique", 0, false);
			if (strcmp(renderTechnique->value(), "Forward") == 0) {
				if (strcmp(api->value(), "Vulkan") == 0) {
					m_pRenderTechnique = new ForwardRenderer;
				}
				else {
					throw std::runtime_error("API not supported : " + std::string(api->value()));
				}
			}
			else if (renderTechnique->value() == "Deferred") {
				throw std::runtime_error("RenderTechnique not implemented : " + std::string(renderTechnique->value()));
			}
			else if (renderTechnique->value() == "Raytracing") {
				throw std::runtime_error("RenderTechnique not implemented : " + std::string(renderTechnique->value()));
			}
			else {
				throw std::runtime_error("RenderTechnique not supported : " + std::string(renderTechnique->value()));
			}
		}
	}

	void Engine::setup(RenderWindow* pWindow, const std::filesystem::path& configPath) {
	
		
		loadFromFile(configPath);
		m_pRenderTechnique->init(pWindow, true);
		m_currentScene = nullptr;


		/*
		Camera* cam = newCamera(pWindow); // DefaultCamera
		cam->setPosition(glm::vec3(0, 0, 1));
		cam->lookAt(glm::vec3(0, 0, 0));
		addActiveCamera(cam);
		*/


	}

	void Engine::update(float dt) {
		if (m_currentScene)
			m_currentScene->update(dt);
	}

	void Engine::recordImGui() {
		m_frameTime.start = std::chrono::high_resolution_clock::now();
		m_pRenderTechnique->beginFrameImGUI();
	}

	void Engine::draw() {
		m_pRenderTechnique->draw(m_currentScene);
		m_frameTime.cpu = std::chrono::high_resolution_clock::now() - m_frameTime.start;
	}

	void Engine::cleanup() {

		ResourceManager::cleanup();
		m_pRenderTechnique->cleanup();
		delete m_pRenderTechnique;
	}

	std::chrono::duration<double, std::milli> Engine::getCPUFrameTime() const {
		return m_frameTime.cpu;
	}

	IRenderTechnique* Engine::getRenderTechnique() const {
		return m_pRenderTechnique;
	}

	Scene& Engine::getScene(const std::string& name) {
		return m_scenes[std::hash<std::string>()(name)];
	}

	Scene* Engine::getCurrentScene() const {
		return m_currentScene;
	}


	void Engine::setScene(const std::string& name) {
		setScene(m_scenes[std::hash<std::string>()(name)]);
	}

	void Engine::setScene(Scene& scene) {
		m_currentScene = &scene;
	}

}

