#include "pch.h"
#include "Engine.h"

#include "Graphics\RenderTechniques\ForwardPlus.h"
#include "Graphics\RenderLayers\ImGuiRenderLayer.h"


#include "Tools\MemoryChecker.h"

#include <simdjson.h>

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

	void Engine::registerMath() {
		{
			auto type = LuaAccessable::registerType<Vector3>("Vec3",
				sol::constructors<Vector3(float, float, float), Vector3(float), Vector3()>(),
				sol::meta_function::addition, &Vector3::operator+=,
				sol::meta_function::subtraction, &Vector3::operator-=,
				sol::meta_function::multiplication, [](const Vector3& self, sol::lua_value value) -> Vector3 {
					if (value.is<float>()) {
						return self * value.as<float>();
					}
					else if (value.is<Vector3>()) {
						return self * value.as<Vector3>();
					}
					throw std::runtime_error("Vec3 __mul: Unmatched type");
				},
				sol::meta_function::division, [](const Vector3& self, sol::lua_value value) -> Vector3 {
					if (value.is<float>()) {
						return self / value.as<float>();
					}
					else if (value.is<Vector3>()) {
						return self / value.as<Vector3>();
					}
					throw std::runtime_error("Vec3 __div: Unmatched type");
				},

					sol::meta_function::length, &Vector3::length,
					sol::meta_function::to_string, &Vector3::toString
					);
			type["x"] = &Vector3::x;
			type["y"] = &Vector3::y;
			type["z"] = &Vector3::z;
			
			type["xy"] = sol::readonly_property([](const Vector3& self) { return Vector2(self.x, self.y); });

			type["unwrap"] = [](const Vector3& self) { return std::tuple(self.x, self.y, self.z); };

			type["length"] = &Vector3::length;
			type["cross"] = [](const Vector3& v1, const Vector3& v2) -> Vector3 {
				return glm::cross(v1, v2);
			};
			type["dot"] = [](const Vector3& v1, const Vector3& v2) -> float {
				return glm::dot((glm::vec3)v1, (glm::vec3)v2);
			};
		}
		{
			auto type = LuaAccessable::registerType<Vector4>("Vec4",
				sol::constructors<Vector4(float, float, float, float), Vector4(float), Vector4(), Vector4(const Vector3&)>(),
				sol::meta_function::addition, &Vector4::operator+=,
				sol::meta_function::subtraction, &Vector4::operator-=,
				sol::meta_function::multiplication, [](const Vector4& self, sol::lua_value value) {
					if (value.is<float>()) {
						return self * value.as<float>();
					}
					else if (value.is<Vector4>()) {
						return self * value.as<Vector4>();
					}
					throw std::runtime_error("Vec4 __mul: Unmatched type");
				},
				sol::meta_function::division, [](const Vector4& self, sol::lua_value value) {
					if (value.is<float>()) {
						return self / value.as<float>();
					}
					else if (value.is<Vector4>()) {
						return self / value.as<Vector4>();
					}
					throw std::runtime_error("Vec4 __div: Unmatched type");
				},

				sol::meta_function::length, &Vector4::length,
				sol::meta_function::to_string, &Vector4::toString
			);

			type["x"] = &Vector4::x;
			type["y"] = &Vector4::y;
			type["z"] = &Vector4::z;
			type["w"] = &Vector4::w;


			type["r"] = &Vector4::x;
			type["g"] = &Vector4::y;
			type["b"] = &Vector4::z;
			type["a"] = &Vector4::w;

			type["xyz"] = sol::readonly_property([](const Vector4& self) { return Vector3(self.x, self.y, self.z); });
			type["xy"] = sol::readonly_property([](const Vector4& self) { return Vector2(self.x, self.y); });

			type["unwrap"] = [](const Vector4& self) { return std::tuple(self.x, self.y, self.z, self.w); };


			type["length"] = &Vector4::length;
			type["dot"] = [](const Vector4& v1, const Vector4& v2) -> float {
				return glm::dot((glm::vec4)v1, (glm::vec4)v2);
			};
		}

		{
			auto type = LuaAccessable::registerType<Vector2>("Vec2",
				sol::constructors<Vector2(float, float), Vector2(float), Vector2()>(),
				sol::meta_function::addition, &Vector2::operator+=,
				sol::meta_function::subtraction, &Vector2::operator-=,
				sol::meta_function::multiplication, [](const Vector2& self, sol::lua_value value) {
					if (value.is<float>()) {
						return self * value.as<float>();
					}
					else if (value.is<Vector2>()) {
						return self * value.as<Vector2>();
					}
					throw std::runtime_error("Vec2 __mul: Unmatched type");
				},
				sol::meta_function::division, [](const Vector2& self, sol::lua_value value) {
					if (value.is<float>()) {
						return self / value.as<float>();
					}
					else if (value.is<Vector2>()) {
						return self / value.as<Vector2>();
					}
					throw std::runtime_error("Vec2 __div: Unmatched type");
				},

				sol::meta_function::length, &Vector2::length,
				sol::meta_function::to_string, &Vector2::toString
			);

			type["x"] = &Vector2::x;
			type["y"] = &Vector2::y;


			type["unwrap"] = [](const Vector2& self) { return std::tuple(self.x, self.y); };


			type["length"] = &Vector2::length;
			type["dot"] = [](const Vector2& v1, const Vector2& v2) -> float {
				return glm::dot((glm::vec2)v1, (glm::vec2)v2);
			};
		}

		{
			auto type = LuaAccessable::registerType<glm::quat>("Quat");
			type["rotate"] = [](glm::quat& self, float angle, Vector3 axis) {
				self = glm::rotate(self, glm::radians(angle), axis);
			};
			type["Identity"] = &glm::quat_identity<float, glm::packed_highp>;
		}
	}

	void Engine::reg() {
		
		registerMath();

		auto& lua = LuaAccessable::getState();
		lua["setScene"] = [&](const sol::lua_value& scene) {
			if (scene.is<std::string>()) {
				setScene(scene.as<std::string>());
				return;
			}
			setScene(scene.as<Scene>());
		};

		lua["getScene"] = [&](sol::lua_value sceneName) {
			if (sceneName.is<sol::nil_t>()) {
				return getCurrentScene();
			}
			return &getScene(sceneName.as<std::string>());
		};
		


		/*
		{
			auto type = LuaAccessable::registerType<Color>("Color");
			type["r"] = &Color::r;
			type["g"] = &Color::g;
			type["b"] = &Color::b;
			type["a"] = &Color::a;

		}
		*/

	}

	void Engine::onWindowResize(Extent newExtent) {
		publish<sa::engine_event::WindowResized>(newExtent);
		m_windowExtent = newExtent;
	}

	void Engine::setup(sa::RenderWindow* pWindow, bool enableImgui) {
		SA_PROFILE_FUNCTION();

		m_currentScene = nullptr;
		m_pWindow = pWindow;
		m_isImGuiRecording = false;

		registerAllComponents();

		reg();
		Scene::reg();
		Entity::reg();
		
		if (pWindow) {
			m_renderPipeline.create(pWindow, new ForwardPlus(!enableImgui));
			if (enableImgui)
				m_renderPipeline.pushLayer(new ImGuiRenderLayer);

			pWindow->setResizeCallback(std::bind(&Engine::onWindowResize, this, std::placeholders::_1));
			m_windowExtent = pWindow->getCurrentExtent();

			on<engine_event::WindowResized>([&](engine_event::WindowResized& e, Engine& emitter) {
				m_renderPipeline.onWindowResize(e.newExtent);

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
		// set scene silently
		//m_currentScene = &getScene("MainScene");

		m_isSetup = true;
	}

	
	void Engine::init() {
		SA_PROFILE_FUNCTION();

		if (!m_currentScene) // no scene has been set
			setScene("Default Scene");
	}

	void Engine::update(float dt) {
		SA_PROFILE_FUNCTION();
		m_currentScene->update(dt);
	}

	void Engine::cleanup() {
		SA_PROFILE_FUNCTION();
		
		m_scenes.clear();
	}

	void Engine::recordImGui() {
		SA_PROFILE_FUNCTION();
		m_renderPipeline.beginFrameImGUI();
		m_isImGuiRecording = true;
	}

	void Engine::draw() {
		SA_PROFILE_FUNCTION();

		if (!m_pWindow)
			return;

		m_renderPipeline.render(getCurrentScene());
		m_isImGuiRecording = false;
	}

	std::chrono::duration<double, std::milli> Engine::getCPUFrameTime() const {
		return m_frameTime.cpu;
	}
	
	const RenderPipeline& Engine::getRenderPipeline() const {
		return m_renderPipeline;
	}

	Scene& Engine::getScene(const std::string& name) {
		auto [it, success] = m_scenes.emplace(name, name);
		if (success) {
			it->second.on<scene_event::SceneRequest>([&](const sa::scene_event::SceneRequest e, Scene&){
				setScene(e.sceneName);
			});
		}
		return it->second;
	}

	Scene& Engine::loadSceneFromFile(const std::filesystem::path& path) {
		simdjson::ondemand::parser parser;
		auto json = simdjson::padded_string::load(path.string());
		if (json.error()) {
			throw std::runtime_error("JSON load failed : " + std::string(simdjson::error_message(json.error())));
		}
		simdjson::ondemand::document doc = parser.iterate(json);
		Scene& scene = getScene(std::string(doc["name"].get_string().value()));
		scene.deserialize(&doc);
		return scene;
	}

	void Engine::storeSceneToFile(Scene* pScene, const std::filesystem::path& path) {
		Serializer s;
		pScene->serialize(s);

		std::ofstream file(path);
		file << s.dump();
		file.close();
	}

	Scene* Engine::getCurrentScene() const {
		return m_currentScene;
	}

	void Engine::setScene(const std::string& name) {
		setScene(getScene(name));
	}

	void Engine::setScene(Scene& scene) {
		SA_PROFILE_FUNCTION();
		publish<engine_event::SceneSet>(&scene);
		if (m_currentScene) {
			m_currentScene->unload();
		}
		m_currentScene = &scene;
		if (m_isSetup) {
			publish<engine_event::SceneLoad>(m_currentScene);
			m_currentScene->load();
		}
	}

	std::unordered_map<std::string, Scene>& Engine::getScenes() {
		return m_scenes;
	}

}

