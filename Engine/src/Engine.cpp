#include "pch.h"
#include "Engine.h"

#include <vulkan/vulkan_core.h>

#include "Graphics/RenderTechniques/ForwardPlus.h"
#include "Lua/Ref.h"
#include "Tools/Vector.h"
#include "Tools/Profiler.h"


namespace sa {
	std::filesystem::path Engine::s_shaderDirectory = std::filesystem::current_path();

	void Engine::registerMath() {

		{
			auto type = LuaAccessable::registerType<Vector3>("Vec3",
				sol::constructors<Vector3(float, float, float), Vector3(float), Vector3(), 
				Vector3(const Vector2&), Vector3(const Vector3&), Vector3(const Vector4&)>(),
				sol::meta_function::addition, [](const Vector3& self, const Vector3& other) -> Vector3 { return self + other; },
				sol::meta_function::subtraction, [](const Vector3& self, const Vector3& other) -> Vector3 { return self - other; },
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
			type["normalize"] = [](Vector3& self) {
				self = glm::normalize(self);
			};

			type["serialize"] = [](const Vector3 &self, Serializer& s) {
				s.value("Vec3", (glm::vec3)self);
			};

			type["deserialize"] = [](simdjson::ondemand::object& jsonObject) -> Vector3 {
				return Serializer::DeserializeVec3(&jsonObject);
			};

		}
		{
			auto type = LuaAccessable::registerType<Vector4>("Vec4",
				sol::constructors<Vector4(float, float, float, float), Vector4(float), Vector4(),
				Vector4(const Vector2&), Vector4(const Vector3&), Vector4(const Vector4&)>(),
				sol::meta_function::addition, [](const Vector4& self, const Vector4& other) -> Vector4 { return self + other; },
				sol::meta_function::subtraction, [](const Vector4& self, const Vector4& other) -> Vector4 { return self - other; },
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

			type["normalize"] = [](Vector4& self) {
				self = glm::normalize(self);
			};

			type["serialize"] = [](const Vector4& self, Serializer& s) {
				s.value("Vec4", (glm::vec4)self);
			};

			type["deserialize"] = [](simdjson::ondemand::object& jsonObject) -> Vector4 {
				return Serializer::DeserializeVec4(&jsonObject);
			};
		}

		{
			auto type = LuaAccessable::registerType<Vector2>("Vec2",
				sol::constructors<Vector2(float, float), Vector2(float), Vector2(),
				Vector2(const Vector2&), Vector2(const Vector3&), Vector2(const Vector4&)>(),
				sol::meta_function::addition, [](const Vector2& self, const Vector2& other) -> Vector2 { return self + other; },
				sol::meta_function::subtraction, [](const Vector2& self, const Vector2& other) -> Vector2 { return self - other; },
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

			type["normalize"] = [](Vector2& self) {
				self = glm::normalize(self);
			};

			type["serialize"] = [](const Vector2& self, Serializer& s) {
				s.value("Vec2", (glm::vec2)self);
			};

			type["deserialize"] = [](simdjson::ondemand::object& jsonObject) -> Vector2 {
				return Serializer::DeserializeVec2(&jsonObject);
			};
		}

		{
			auto type = LuaAccessable::registerType<glm::quat>("Quat");
			type["rotate"] = [](glm::quat& self, float angle, const Vector3& axis) {
				self = glm::rotate(self, glm::radians(angle), axis);
			};

			type["lookAt"] = [](glm::quat& self, const Vector3& direction, const Vector3& up) {
				self = glm::quatLookAt(direction, up);
			};

			type["Identity"] = &glm::quat_identity<float, glm::packed_highp>;
			type["serialize"] = [](const glm::quat& self, Serializer& s) {
				s.value("Quat", self);
			};

			type["deserialize"] = [](simdjson::ondemand::object& jsonObject) -> glm::quat {
				return Serializer::DeserializeQuat(&jsonObject);
			};
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
				throw sol::error("[Serialize] First argument is not a table");
			}
			sol::table t = table.as<sol::table>();
			sol::environment& env = thisEnv;
			sa::Entity entity = env["this_entity"];
			std::string scriptName = env["this_name"];
			
			EntityScript* pScript = entity.getScript(scriptName);
			if (!pScript) {
				throw sol::error("[Serialize] No such script! " + scriptName);
			}
			
			for (auto& [key, value] : t) {
				sol::object v = value;
				std::string variableName = key.as<std::string>();
				// if stored load stored value
				if (pScript->serializedData.count(variableName)) {
					value = pScript->serializedData[variableName];
					SA_DEBUG_LOG_INFO("Variable overwritten by serialized data: ", variableName, " = ", sol::type_name(LuaAccessable::getState(), value.get_type()));
				}
				else {
					// else store this value
					pScript->serializedData[variableName] = value;
					SA_DEBUG_LOG_INFO("Serialized data added: ", variableName, " = ", sol::type_name(LuaAccessable::getState(), value.get_type()));
				}
				// initialize variable with appropriate value
				env[variableName] = value;
			}

		};
	}

	void Engine::onWindowResize(Extent newExtent) {
		m_pWindowRenderer->onWindowResize(newExtent);

		trigger<engine_event::WindowResized>(engine_event::WindowResized{ m_windowExtent, newExtent });
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
		Entity::reg();
		Ref::reg();
		
		if (pWindow) {
			m_renderPipeline.create(new ForwardPlus);
			
			setWindowRenderer(new WindowRenderer(m_pWindow));

			pWindow->setResizeCallback(std::bind(&Engine::onWindowResize, this, std::placeholders::_1));
			m_windowExtent = pWindow->getCurrentExtent();
			m_mainRenderTarget.initialize(this, m_pWindow);
		}
		sink<engine_event::SceneSet>().connect<&Engine::onSceneSet>(this);

	}

	void Engine::cleanup() {
		if (m_pWindowRenderer)
			delete m_pWindowRenderer;
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

		RenderContext context = m_pWindow->beginFrame();
		if (!context)
			return;
		
		if (getCurrentScene()) {
			
			bool renderedToMainRenderTarget = false;
			m_currentScene.getAsset()->forEach<comp::Camera>([&](comp::Camera& camera) {
				RenderTarget* pRenderTarget = camera.getRenderTarget();
				if (pRenderTarget) {
					m_renderPipeline.render(context, &camera.camera, pRenderTarget, camera.getSceneCollection());
				}
				else {
					if(!renderedToMainRenderTarget)
						m_renderPipeline.render(context, &camera.camera, &m_mainRenderTarget, camera.getSceneCollection());
					renderedToMainRenderTarget = true;
				}
			});
			
		}
		trigger<engine_event::OnRender>(engine_event::OnRender{ &context, &m_renderPipeline });

		m_pWindowRenderer->render(context, m_mainRenderTarget.getOutputTexture());
		{
			SA_PROFILE_SCOPE("Display");
			m_pWindow->display();
		}
	}

	const RenderPipeline& Engine::getRenderPipeline() const {
		return m_renderPipeline;
	}

	const RenderTarget& Engine::getMainRenderTarget() const {
		return m_mainRenderTarget;
	}

	void Engine::setWindowRenderer(IWindowRenderer* pWindowRenderer) {
		if (m_pWindowRenderer)
			delete m_pWindowRenderer;
		m_pWindowRenderer = pWindowRenderer;
	}

	Scene* Engine::getCurrentScene() const {
		return m_currentScene.getAsset();
	}

	void Engine::setScene(Scene* scene) {
		SA_PROFILE_FUNCTION();

		if (m_currentScene.getAsset() == scene)
			return;

		if (m_currentScene) 
			m_currentScene.getProgress()->wait();

		if (scene) {
			scene->hold();
			scene->getProgress().wait();
		}

		trigger<engine_event::SceneSet>(engine_event::SceneSet{ m_currentScene.getAsset(), scene });

		m_currentScene = scene;

		if(scene)
			scene->release();
		

	}

	void Engine::onSceneSet(engine_event::SceneSet& e) {
		if (e.oldScene) {
			e.oldScene->onRuntimeStop();
		}
		e.newScene->onRuntimeStart();
	}
}

