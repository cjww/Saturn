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

	}

	void Engine::setup(RenderWindow* pWindow, const std::filesystem::path& configPath) {
	
		registerComponents();

		m_currentScene = nullptr;
		loadFromFile(configPath);
		m_pRenderTechnique->init(pWindow, false);


		setScene("Scene");
		Entity e = getCurrentScene()->createEntity();
		e.addComponent<comp::Model>()->modelID = 42;

		/*
		Camera* cam = newCamera(pWindow); // DefaultCamera
		cam->setPosition(glm::vec3(0, 0, 1));
		cam->lookAt(glm::vec3(0, 0, 0));
		addActiveCamera(cam);
		*/

		m_scriptManager.load("test.lua");

		for (auto& script : m_scriptManager.getScripts()) {

			m_currentScene->forEach(script.components, [&](const Entity& entity)
			{
				for (auto& type : script.components) {
					auto metaComp = type.invoke("get", entity);
					

					sol::state& lua = m_scriptManager.getState();
					/*
					script.env.push();
					std::cout << "Stack: " << lua.stack_top() << " : " << lua_typename(lua, lua_type(lua, -1)) << std::endl;

					void** typePtr = (void**)lua_newuserdata(lua, sizeof(void*));
					*typePtr = sol::detail::align_usertype_pointer(metaComp.data());

					std::cout << "Stack: " << lua.stack_top() << " : " << lua_typename(lua, lua_type(lua, -1)) << std::endl;
					sol::metatable mt = lua.script("local c = " + name + ".new() \n return getmetatable(c)");
					mt.push();
					std::cout << "Stack: " << lua.stack_top() << " : " << lua_typename(lua, lua_type(lua, -1)) << std::endl;

					lua_setmetatable(lua, -2);
					std::cout << "Stack: " << lua.stack_top() << " : " << lua_typename(lua, lua_type(lua, -1)) << std::endl;

					lua_setfield(lua, -2, utils::toLower(name).c_str());
					std::cout << "Stack: " << lua.stack_top() << " : " << lua_typename(lua, lua_type(lua, -1)) << std::endl;

					lua_pop(lua, lua_gettop(lua));
					std::cout << "Stack: " << lua.stack_top() << " : " << lua_typename(lua, lua_type(lua, -1)) << std::endl;


					script.env["model"] = lua["Model"]["new"](metaComp.data());
					sol::table m = lua["Model"][sol::metatable_key];
					m.for_each([](sol::object k, sol::object v) {
						std::cout << k.as<std::string>() << std::endl;
					});
					
					//script.env["model"] = metaComp.data();

					lua.script("print(model.id)", script.env);
					*/
					
				}
			});
			script.env.set_on(script.func);
			
			auto ret = script.func();
			if(!ret.valid()) {
				DEBUG_LOG_ERROR(lua_tostring(m_scriptManager.getState(), -1));
			}
			

		}


	}

	void Engine::update(float dt) {
		if (m_currentScene)
			m_currentScene->update(dt);
	}

	void Engine::cleanup() {
		m_pRenderTechnique->cleanup();
		m_pRenderTechnique.reset();
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

}

