#include "pch.h"
#include "Engine.h"

namespace sa {
	std::filesystem::path Engine::s_shaderDirectory = std::filesystem::current_path();

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
		//TODO
		/*
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
		*/

		lua["Serialize"] = [](sol::lua_value table, sol::this_environment thisEnv) {
			if (!table.is<sol::table>()) {
				SA_DEBUG_LOG_ERROR("First argument is not a table");
				return;
			}
			sol::table t = table.as<sol::table>();
			sol::environment& env = thisEnv;
			sa::Entity entity = env["entity"];
			std::string scriptName = env["scriptName"];
			
			EntityScript* pScript = entity.getScript(scriptName);
			if (!pScript) {
				SA_DEBUG_LOG_ERROR("No such script! ", scriptName);
				return;
			}
			
			for (auto& [key, value] : t) {
				sol::object v = value;
				std::string variableName = key.as<std::string>();
				// if stored load stored value
				if (pScript->serializedData.count(variableName)) {
					value = pScript->serializedData[variableName];
				}
				else {
					// else store this value
					pScript->serializedData[variableName] = value;
				}
				// initialize variable with appropriate value
				env[variableName] = value;
			}
			
			
			//SA_DEBUG_LOG_INFO("Serialize");
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
		m_renderPipeline.onWindowResize(newExtent);
		publish<sa::engine_event::WindowResized>(m_windowExtent, newExtent);
		
		/*
		for (auto& [name, scene] : m_scenes) {
			scene.view<comp::Camera>().each([&](comp::Camera& camera) {
				sa::Rect viewport = camera.camera.getViewport();
				// resize viewport but keep relative size to window size
				viewport.extent.width = newExtent.width * viewport.extent.width / (float)m_windowExtent.width;
				viewport.extent.height = newExtent.height * viewport.extent.height / (float)m_windowExtent.height;
				camera.camera.setViewport(viewport);
			});
		}
		*/

		
		m_windowExtent = newExtent;
	}

	const std::filesystem::path& Engine::getShaderDirectory() {
		return s_shaderDirectory;
	}

	void Engine::setShaderDirectory(const std::filesystem::path& path) {
		s_shaderDirectory = std::filesystem::absolute(path);
	}

	void Engine::setup(sa::RenderWindow* pWindow, bool enableImgui) {
		SA_PROFILE_FUNCTION();
		
		m_currentScene = nullptr;
		m_pWindow = pWindow;
		
		registerAllComponents();

		reg();
		Scene::reg();
		Entity::reg();
		
		if (pWindow) {
			m_renderPipeline.create(pWindow, new ForwardPlus);
			
			m_renderPipeline.pushLayer(new MainRenderLayer);
			m_renderPipeline.pushLayer(new BloomRenderLayer);
			if (enableImgui) {
				m_renderPipeline.pushOverlay(new ImGuiRenderLayer);
			}
			

			pWindow->setResizeCallback(std::bind(&Engine::onWindowResize, this, std::placeholders::_1));
			m_windowExtent = pWindow->getCurrentExtent();

		}
		
		on<engine_event::SceneSet>([](engine_event::SceneSet& e, Engine&) {
			if (e.oldScene) {
				e.oldScene->onRuntimeStop();
			}
			e.newScene->onRuntimeStart();
		});
		
	}

	void Engine::cleanup() {
		AssetManager::get().clear();
	}

	void Engine::recordImGui() {
		SA_PROFILE_FUNCTION();
		m_renderPipeline.beginFrameImGUI();
	}

	void Engine::draw() {
		SA_PROFILE_FUNCTION();

		if (!m_pWindow)
			return;

		RenderContext context = m_renderPipeline.beginScene(getCurrentScene());
		if (context) {
			if(m_currentScene)
				publish<engine_event::OnRender>(&context, &m_renderPipeline);
			m_renderPipeline.endScene();
		}
	}

	std::chrono::duration<double, std::milli> Engine::getCPUFrameTime() const {
		return m_frameTime.cpu;
	}
	
	const RenderPipeline& Engine::getRenderPipeline() const {
		return m_renderPipeline;
	}

	Scene* Engine::getCurrentScene() const {
		return m_currentScene;
	}

	void Engine::setScene(Scene* scene) {
		SA_PROFILE_FUNCTION();
		publish<engine_event::SceneSet>(m_currentScene, scene);
		m_currentScene = scene;	
	}
}

