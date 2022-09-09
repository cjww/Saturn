#include "pch.h"
#include "Engine.h"

#include "Graphics\ForwardRenderer.h"
#include "Graphics\ForwardPlus.h"


#include "Tools\MemoryChecker.h"

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
					//m_pRenderTechnique = std::make_unique<ForwardRenderer>();
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
			auto type = LuaAccessable::luaReg<Vector3>("Vec3", sol::constructors<Vector3(float, float, float), Vector3(float)>());
			type["x"] = &Vector3::x;
			type["y"] = &Vector3::y;
			type["z"] = &Vector3::z;
			
		}
		{
			auto type = LuaAccessable::luaReg<glm::quat>("Quat");
			type["rotate"] = [](glm::quat& self, float angle, Vector3 axis) {
				self = glm::rotate(self, glm::radians(angle), axis);
			};
			type["Identity"] = &glm::quat_identity<float, glm::packed_highp>;
		}
		/*
		{
			//auto type = m_scriptManager.registerType<Vector3>();
			auto type = LuaAccessable::luaReg<Color>("Color");
			type["r"] = &Color::r;
			type["g"] = &Color::g;
			type["b"] = &Color::b;
			type["a"] = &Color::a;

		}
		*/

	}

	void Engine::onWindowResize(Extent newExtent) {
		publish<sa::event::WindowResized>(newExtent);
		m_windowExtent = newExtent;
	}

	void Engine::setup(sa::RenderWindow* pWindow, bool enableImgui) {
		SA_PROFILE_FUNCTION();

		m_currentScene = nullptr;
		m_pRenderTechnique = nullptr;

		registerComponents();
		
		if (pWindow) {
			m_pRenderTechnique = new ForwardPlus();
			m_pRenderTechnique->init(pWindow, enableImgui);
			pWindow->setResizeCallback(std::bind(&Engine::onWindowResize, this, std::placeholders::_1));
			m_windowExtent = pWindow->getCurrentExtent();

			on<event::WindowResized>([&](event::WindowResized& e, Engine& emitter) {
				m_pRenderTechnique->onWindowResize(e.newExtent);
				for (const auto& [id, scene] : m_scenes) {
					for (auto& cam : scene.getActiveCameras()) {
						Rect viewport = cam->getViewport();
						// resize viewport but keep relative size to window size
						viewport.extent.width = e.newExtent.width * viewport.extent.width / m_windowExtent.width;
						viewport.extent.height = e.newExtent.height * viewport.extent.height / m_windowExtent.height;

						cam->setViewport(viewport);
					}
				}

			});
		}
		setScene("MainScene");
	

		m_isSetup = true;
	}

	
	void Engine::init() {
		SA_PROFILE_FUNCTION();

		if (m_currentScene) {
			m_currentScene->init();
			m_scriptManager.init(m_currentScene);
		}

	}

	void Engine::update(float dt) {
		SA_PROFILE_FUNCTION();

		if (m_currentScene) {
			m_scriptManager.update(dt, m_currentScene);
			m_currentScene->update(dt);
		}

	}

	void Engine::cleanup() {
		SA_PROFILE_FUNCTION();

		if (m_pRenderTechnique) {
			m_pRenderTechnique->cleanup();
			delete m_pRenderTechnique;
		}
		m_scenes.clear();
	}

	void Engine::recordImGui() {
		SA_PROFILE_FUNCTION();

		if (m_pRenderTechnique)
			m_pRenderTechnique->beginFrameImGUI();
	}

	void Engine::draw() {
		SA_PROFILE_FUNCTION();

		if (m_pRenderTechnique)
			m_pRenderTechnique->draw(m_currentScene);	
	}

	std::chrono::duration<double, std::milli> Engine::getCPUFrameTime() const {
		return m_frameTime.cpu;
	}

	IRenderTechnique* Engine::getRenderTechnique() const {
		return m_pRenderTechnique;
	}

	Scene& Engine::getScene(const std::string& name) {
		size_t size = m_scenes.size();
		Scene& scene = m_scenes[std::hash<std::string>()(name)];
		if (size != m_scenes.size()) {
			// first use

		}
		return scene;
	}

	Scene* Engine::getCurrentScene() const {
		return m_currentScene;
	}

	void Engine::setScene(const std::string& name) {
		setScene(getScene(name));
	}

	void Engine::setScene(Scene& scene) {
		SA_PROFILE_FUNCTION();

		m_currentScene = &scene;
		if(m_isSetup)
			init();
	}

	void Engine::createSystemScript(const std::string& name) {
		m_scriptManager.load(name);
	}

}

