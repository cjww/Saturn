#include "pch.h"
#include "Engine.h"

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
					m_pRenderTechnique = std::make_unique<ForwardRenderer>();
				}
				else if (strcmp(api->value(), "SFML") == 0) {
					m_pRenderTechnique = std::make_unique<SFML2DRenderer>();
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

	void Engine::registerComponents() {
		// TODO Could this be automated?
		{
			auto type = m_scriptManager.registerType<Vector3>();
			type["x"] = &Vector3::x;
			type["y"] = &Vector3::y;
			type["z"] = &Vector3::z;
		}


		{
			registerComponentType<comp::Model>();
			auto type = m_scriptManager.registerComponent<comp::Model>();
			type["id"] = &comp::Model::modelID;
		}

		{
			registerComponentType<comp::Transform>();
			auto type = m_scriptManager.registerComponent<comp::Transform>();
			type["position"] = &comp::Transform::position;
			type["rotation"] = &comp::Transform::rotation;
			type["scale"] = &comp::Transform::scale;
		}
		{
			registerComponentType<comp::Script>();
			auto type = m_scriptManager.registerComponent<comp::Script>();
			type["__index"] = [](const comp::Script& script, const std::string& key) {
				return (*script.env)[key];
			};
		}

		
		{
			m_scriptManager.registerEntityType();
		}

	}

	void Engine::setup(RenderWindow* pWindow, const std::filesystem::path& configPath) {
	
		registerComponents();

		m_currentScene = nullptr;
		loadFromFile(configPath);
		m_pRenderTechnique->init(pWindow, false);

		setScene("Scene");
		
		/*
		Camera* cam = newCamera(pWindow); // DefaultCamera
		cam->setPosition(glm::vec3(0, 0, 1));
		cam->lookAt(glm::vec3(0, 0, 0));
		addActiveCamera(cam);
		*/

	}

	void Engine::init() {
		m_scriptManager.init(m_currentScene);
	}

	void Engine::update(float dt) {
		if (m_currentScene) {
			m_scriptManager.update(dt, m_currentScene);
			m_currentScene->update(dt);
		}
	}

	void Engine::cleanup() {
		m_pRenderTechnique->cleanup();
		m_pRenderTechnique.reset();
		m_scenes.clear();
	}

	void Engine::recordImGui() {
		m_frameTime.start = std::chrono::high_resolution_clock::now();
		m_pRenderTechnique->beginFrameImGUI();
	}

	void Engine::draw() {
		m_pRenderTechnique->draw(m_currentScene);
		m_frameTime.cpu = std::chrono::high_resolution_clock::now() - m_frameTime.start;
	}

	std::chrono::duration<double, std::milli> Engine::getCPUFrameTime() const {
		return m_frameTime.cpu;
	}

	IRenderTechnique* Engine::getRenderTechnique() const {
		return m_pRenderTechnique.get();
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

	void Engine::createSystemScript(const std::string& name) {
		m_scriptManager.load(name);
	}

}

