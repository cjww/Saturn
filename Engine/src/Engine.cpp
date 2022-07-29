#include "pch.h"
#include "Engine.h"

#include "Graphics\ForwardRenderer.h"

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
		{
			//auto type = m_scriptManager.registerType<Vector3>();
			auto type = LuaAccessable::luaReg<Vector3>();
			type["x"] = &Vector3::x;
			type["y"] = &Vector3::y;
			type["z"] = &Vector3::z;
		}

	}

	Scene& Engine::setup(sa::RenderWindow* pWindow, const std::filesystem::path& configPath) {
	
		registerComponents();

		m_currentScene = nullptr;
		if(!configPath.empty())
			loadFromFile(configPath);
		if (pWindow)
		{
			m_pRenderTechnique = std::make_unique<ForwardRenderer>();
			m_pRenderTechnique->init(pWindow, false);
		}
		setScene("MainScene");
	

		/*
		Camera* cam = newCamera(pWindow); // DefaultCamera
		cam->setPosition(glm::vec3(0, 0, 1));
		cam->lookAt(glm::vec3(0, 0, 0));
		addActiveCamera(cam);
		*/

		m_isSetup = true;
		return *m_currentScene;
	}

	
	void Engine::init() {
		if (m_currentScene) {
			m_currentScene->init();
			m_scriptManager.init(m_currentScene);
		}

	}

	void Engine::update(float dt) {

		if (m_currentScene) {
			m_scriptManager.update(dt, m_currentScene);
			m_currentScene->update(dt);
			if (m_pRenderTechnique->getCurrentExtent().width > 1.f || m_pRenderTechnique->getCurrentExtent().height > 1.f) {
				for (auto& camera : m_currentScene->getActiveCameras()) {
					camera->setViewport(sa::Rect{ { 0, 0 }, m_pRenderTechnique->getCurrentExtent() });
				}
			}
		
		}

	}

	void Engine::cleanup() {
		if (m_pRenderTechnique) {
			m_pRenderTechnique->cleanup();
			m_pRenderTechnique.reset();
		}
		m_scenes.clear();
	}

	void Engine::recordImGui() {
		if (m_pRenderTechnique)
			m_pRenderTechnique->beginFrameImGUI();
	}

	void Engine::draw() {
		if (m_pRenderTechnique)
			m_pRenderTechnique->draw(m_currentScene);	
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
		setScene(getScene(name));
	}

	void Engine::setScene(Scene& scene) {
		m_currentScene = &scene;
		if(m_isSetup)
			init();
	}

	void Engine::createSystemScript(const std::string& name) {
		m_scriptManager.load(name);
	}

}

