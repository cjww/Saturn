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
		/*
		Camera* cam = newCamera(pWindow); // DefaultCamera
		cam->setPosition(glm::vec3(0, 0, 1));
		cam->lookAt(glm::vec3(0, 0, 0));
		addActiveCamera(cam);
		*/


	}

	void Engine::update() {

	}

	void Engine::recordImGui() {
		m_frameTime.start = std::chrono::high_resolution_clock::now();
		m_pRenderTechnique->beginFrameImGUI();
	}

	void Engine::draw() {
		m_pRenderTechnique->draw();
		m_frameTime.cpu = std::chrono::high_resolution_clock::now() - m_frameTime.start;
	}

	void Engine::cleanup() {
		for (auto& cam : m_cameras) {
			delete cam;
		}

		ECSCoordinator::cleanup();
		ResourceManager::cleanup();
		m_pRenderTechnique->cleanup();
		delete m_pRenderTechnique;
	}

	std::chrono::duration<double, std::milli> Engine::getCPUFrameTime() const {
		return m_frameTime.cpu;
	}

	Camera* Engine::newCamera() {
		m_cameras.push_back(new Camera());
		return m_cameras.back();
	}

	Camera* Engine::newCamera(const RenderWindow* pWindow) {
		m_cameras.push_back(new Camera(pWindow));
		return m_cameras.back();
	}


	void Engine::addActiveCamera(Camera* camera) {
		m_pRenderTechnique->addCamera(camera);
	}

	void Engine::removeActiveCamera(Camera* camera) {
		m_pRenderTechnique->removeCamera(camera);
	}
}

