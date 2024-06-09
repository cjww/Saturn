#pragma once
#include "LuaAccessable.h"
#include "ECS/Components/Light.h"
#include <ECS/Entity.h>

#include "Engine.h"
#include "Ref.h"
#include "ECS/Components/Model.h"
#include "ECS/Components/Name.h"
#include "ECS/Components/Transform.h"

#include <simdjson.h>


namespace sa {


	template<typename Comp, std::enable_if_t<std::is_base_of_v<sa::ComponentBase, std::decay_t<Comp>>, bool> = true>
	inline void registerComponentLua() {
		if (LuaAccessable::registerType<Comp>()) {
			LuaAccessable::registerComponent<Comp>();

			LuaAccessable::getState()[getComponentName<Comp>()]["Get"] = [](const Entity& entity) {
				return entity.getComponent<Comp>();
			};
			auto& type = LuaAccessable::userType<Entity>();
			std::string name = getComponentName<Comp>();
			std::string varName = utils::toLower(name);
			type[varName] = sol::property(
				[=](const Entity& self) -> sol::lua_value {
					MetaComponent metaComp = self.getComponent(name);
					return LuaAccessable::cast(metaComp);
				},
				[=](Entity& self, sol::lua_value component) {
					if (!component.is<sol::nil_t>()) {
						// Add component
						MetaComponent mc = self.addComponent(name);
						LuaAccessable::copy(mc, component);
						return;
					}
					self.removeComponent(name);
				});

			SA_DEBUG_LOG_INFO("Registered Lua property for ", getComponentName<Comp>());
		}
	}


	// ----------------- Components -----------------
	template<>
	inline bool sa::LuaAccessable::registerType<comp::Light>() {
		auto& type = userType<comp::Light>("",
			sol::constructors<comp::Light()>());

		type["intensity"] = sol::property([](const comp::Light& self) {return self.values.color.a; }, [](comp::Light& self, float value) { self.values.color.a = value; });
		type["attenuationRadius"] = sol::property([](const comp::Light& self) {return self.values.position.w; }, [](comp::Light& self, float value) { self.values.position.w = value; });
		type["color"] = sol::property(
			[](const comp::Light& light) {return sa::Vector3(light.values.color.r, light.values.color.g, light.values.color.b); },
			[](comp::Light& self, const sa::Vector3& color) {self.values.color = sa::Color{ color.x, color.y, color.z, self.values.color.a }; }
		);
		return true;
	}

	template<>
	inline bool sa::LuaAccessable::registerType<comp::Model>() {
		auto& type = userType<comp::Model>("",
			sol::constructors<comp::Model()>()
			);

		//type["id"] = &comp::Model::model.getID();
		type["setAsset"] = [&](comp::Model& self, sa::ModelAsset* pAsset) {
			self.model = pAsset;
		};
		type["getAsset"] = [&](const comp::Model& self) {
			return self.model.getAsset();
		};
		return true;
	}

	template<>
	inline bool sa::LuaAccessable::registerType<comp::Transform> () {
		auto type = userType<comp::Transform>();
		type["position"] = sol::property(
			[](comp::Transform& self) -> sa::Vector3& {
				if (self.hasParent)
					return self.relativePosition;
				return self.position;
			},
			[](comp::Transform& self, const sa::Vector3& value) {
				if (self.hasParent) {
					self.relativePosition = value;
					return;
				}
				self.position = value;
			}
			);
		type["rotation"] = &comp::Transform::rotation;
		type["scale"] = &comp::Transform::scale;
		return true;
	}

	// ----------------- Other -----------------
	template<>
	inline bool LuaAccessable::registerType<Engine>() {
		auto& lua = getState();
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
			std::unordered_set<std::string> currentSerializedData;
			for (const auto& [key, value] : pScript->serializedData) {
				currentSerializedData.insert(key);
			}
			for (auto& [key, value] : t) {
				sol::object v = value;
				std::string variableName = key.as<std::string>();
				currentSerializedData.erase(variableName);
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
			for (const auto& key : currentSerializedData) {
				pScript->serializedData.erase(key);
			}
		};

		return true;
	}

	template<>
	inline bool LuaAccessable::registerType<Entity>() {
		auto& type = userType<Entity>();

		type["id"] = sol::readonly_property(&Entity::operator entt::id_type);
		type["name"] = sol::property(
			[](const Entity& e) -> std::string { return e.getComponent<comp::Name>()->name; },
			[](const Entity& e, const std::string& str) { e.getComponent<comp::Name>()->name = str; }
		);

		type["addScript"] = [](Entity& self, const std::string& filepath) { self.addScript(filepath); };
		type["removeScript"] = &Entity::removeScript;

		type["addComponent"] = [](Entity& self, const std::string& name) {
			self.addComponent(name);
		};
		type["removeComponent"] = [](Entity& self, const std::string& name) {
			self.removeComponent(name);
		};

		type["update"] = [](Entity& self, const sol::table& type) {
			const std::string name = LuaAccessable::getState()[type];
			self.updateComponent(name);
		};

		type["clone"] = [](Entity& self) { return self.clone(); };
		type["destroy"] = &Entity::destroy;

		type["isNull"] = &Entity::isNull;

		type["parent"] = sol::property(
			[](const Entity& self) -> sol::lua_value {
				const Entity& parent = self.getParent();
				if (parent.isNull()) {
					return sol::nil;
				}
				return parent;
			},
			[](Entity& self, const sol::lua_value& parent) {
				if (parent.is<sol::nil_t>()) {
					self.orphan();
					return;
				}
				self.setParent(parent.as<Entity>());
			}

			);

		type["__index"] = [](Entity& self, std::string key) -> sol::lua_value {
			EntityScript* pScript = self.getScript(key);
			if (!pScript)
				return sol::nil;

			return pScript->env;
		};

		type[sol::meta_function::to_string] = [](const Entity& self) { return self.toString(); };

		type["Get"] = [](const Entity& entity) -> const Entity& {
			return entity;
		};
		return true;
	}

	template<>
	inline bool LuaAccessable::registerType<Ref>() {
		auto& type = userType<Ref>("Ref",
			sol::call_constructor,
			sol::constructors<Ref(const std::string&, const Entity&), Ref(const sol::table&, const Entity&), Ref(const sol::object&, const sol::nil_t&)>());

		type["hasReference"] = &Ref::hasReference;

		type["__index"] = [](Ref& self, const sol::lua_value& key) -> sol::lua_value {
			sol::lua_value v = self.m_retriveFunction()[key];
			if (v.is<sol::function>())
				throw sol::error("Referencing functions from Ref is not allowed. Use ref.value." + key.as<std::string>() + " instead");
			return v;
		};

		type["value"] = sol::property([](Ref& self) -> sol::lua_value {
			return self.m_retriveFunction();
			});


		type["serialize"] = [](const Ref& self, Serializer& s) {
			self.m_serializeFunc(self, s);
		};

		type["deserialize"] = [](simdjson::ondemand::object& jsonObject, sol::this_state ts, sol::this_environment te) -> sol::lua_value {
			const sol::state_view& lua = ts;
			sol::environment& env = te;

			sol::lua_value type(lua, sol::nil);
			sol::lua_value value(lua, sol::nil);

			auto typeResult = jsonObject.find_field("type");
			if (typeResult.error() != simdjson::error_code::SUCCESS)
				throw std::runtime_error("[Ref.deserialize]: Could not find json field \"type\"");

			std::string_view typeStr = typeResult.get_string().take_value();

			if (lua[typeStr] != sol::nil)
				type = lua[typeStr];
			else
				type = typeStr;

			auto valueResult = jsonObject.find_field("value");
			if (valueResult.error() == simdjson::error_code::SUCCESS) {
				if (valueResult.is_integer()) {
					int64_t id = valueResult.get_int64().take_value();
					Scene* pScene = env["this_scene"];
					Entity entity(pScene, static_cast<entt::entity>(id));
					value = entity;
				}
				else
					value = static_cast<uint64_t>(valueResult.get_int64_in_string().take_value());
			}

			return lua["Ref"](type, value);
		};

		return true;
	}

	// ----------------- Assets -----------------
	template<>
	inline bool LuaAccessable::registerType<AssetManager>() {
		auto& type = userType<AssetManager>("AssetManager",
			sol::no_constructor);
		type["getCube"] = &AssetManager::getCube;
		type["getQuad"] = &AssetManager::getQuad;
		type["getSphere"] = &AssetManager::getSphere;

		//type["getAsset"] = &AssetManager::getAsset;
		type["findAssetByName"] = [](AssetManager& self, const std::string& name) {
			return self.findAssetByName(name);
		};
		type["findAssetByPath"] = [](AssetManager& self, const std::string& path) {
			return self.findAssetByPath(path);
		};

		getState()["AssetManager"] = &AssetManager::Get();

		return true;
	}

	template<typename AssetType>
	inline void registerAssetBase(sol::usertype<AssetType>& type) {
		type["hold"] = &AssetType::hold;
		type["write"] = &AssetType::write;
		type["release"] = &AssetType::release;

		type["isLoaded"] = &AssetType::isLoaded;

		type["clone"] = [](const AssetType& self, const std::string& name) { return self.clone(name); };


		type["name"] = sol::property(&AssetType::getName);
		type["id"] = sol::property(&AssetType::getID);
		type["referenceCount"] = sol::property(&AssetType::getReferenceCount);
		type["path"] = sol::property([](const AssetType& self) { return self.getAssetPath().generic_string(); });

		/*
		type["progress"] = sol::property([](const AssetType& self) -> const ProgressView<bool>* {
			return self.getProgress();
		});
		*/

		// load status
		type["completion"] = sol::property([](const AssetType& self) {
			return self.getProgress().getCompletion();
		});
		type["allCompletion"] = sol::property([](const AssetType& self) {
			return self.getProgress().getAllCompletion();
		});
		type["wait"] = [](const AssetType& self, sol::object timeout) {
			if(timeout.get_type() == sol::type::number) {
				self.getProgress().wait(std::chrono::seconds(timeout.as<long long>()));
				return;
			}
			self.getProgress().wait();
		};
		type["waitAll"] = [](const AssetType& self, sol::object timeout) {
			if (timeout.get_type() == sol::type::number) {
				self.getProgress().waitAll(std::chrono::seconds(timeout.as<long long>()));
				return;
			}
			self.getProgress().waitAll();
		};

		type["isDone"] = [](const AssetType& self, sol::object timeout) {
			return self.getProgress().isDone();
		};
		type["isAllDone"] = [](const AssetType& self, sol::object timeout) {
			return self.getProgress().isAllDone();
		};

	}

	template<>
	inline bool LuaAccessable::registerType<Asset>() {
		{
			auto& type = userType<ProgressView<bool>>("AssetProgressView",
				sol::no_constructor);
			type["getCompletion"] = &ProgressView<bool>::getCompletion;
			type["isDone"] = &ProgressView<bool>::isDone;
			type["isAllDone"] = &ProgressView<bool>::isAllDone;
			type["value"] = sol::property(&ProgressView<bool>::getValue);

		}
		

		auto& type = userType<Asset>("Asset",
			sol::no_constructor);
		registerAssetBase(type);

		return true;
	}

	template<>
	inline bool LuaAccessable::registerType<Scene>() {
		auto type = userType<Scene>("Scene",
			sol::no_constructor,
			sol::base_classes, sol::bases<Asset>());

		type["findEntitiesByName"] = [](Scene& self, const std::string& name) {
			std::vector<Entity> entities;
			self.forEachEntity([&](const Entity& e) {
				if (e.getComponent<comp::Name>()->name == name) {
					entities.emplace_back(e);
				}
				});
			return sol::as_table(entities);
		};

		type["createEntity"] = [](Scene& self, const sol::object& name) {
			switch (name.get_type()) {
			case sol::type::nil:
				return self.createEntity();
			case sol::type::string:
				return self.createEntity(name.as<std::string>());
			default:
				throw sol::error("Unexpected parameter '" + sol::type_name(getState(), name.get_type()) + "'");
			}

		};

		return true;
	}

	template<>
	inline bool LuaAccessable::registerType<ModelAsset>() {
		{
			
			auto& vertexType = userType<VertexNormalUV>();
			vertexType["position"] = &VertexNormalUV::position;
			vertexType["normal"] = &VertexNormalUV::normal;
			vertexType["texCoord"] = &VertexNormalUV::texCoord;


			auto& type = userType<Mesh>();
			type["material"] = sol::property([](const Mesh& self) { return self.material.getAsset(); },
				[](Mesh& self, Material* material) { self.material = material; });
			type["vertices"] = &Mesh::vertices;
			type["indices"] = &Mesh::indices;
		}

		auto& type = userType<ModelAsset>("ModelAsset",
			sol::no_constructor);
		registerAssetBase(type);

		type["meshes"] = sol::property(
			[](ModelAsset& self) -> std::vector<Mesh>& {
				return self.data.meshes;
			},
			[](ModelAsset& self, const std::vector<Mesh>& meshes) {
				self.data.meshes = meshes;
			});

		return true;
	}

	template<>
	inline bool LuaAccessable::registerType<Material>() {
		auto& type = userType<Material>("Material",
			sol::no_constructor);
		registerAssetBase(type);

		type["roughness"] = sol::property(
			[](const Material& self) { return self.values.roughness; }, 
			[](Material& self, float value) { self.values.roughness = value; });

		type["metallic"] = sol::property(
			[](const Material& self) { return self.values.metallic; },
			[](Material& self, float value) { self.values.metallic = value; });
		


		return true;
	}

	// ----------------- Math -----------------

	template<>
	inline bool LuaAccessable::registerType<Vector3>() {
		auto& type = userType<Vector3>("Vec3",
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

		type["serialize"] = [](const Vector3& self, Serializer& s) {
			s.value("Vec3", (glm::vec3)self);
		};

		type["deserialize"] = [](simdjson::ondemand::object& jsonObject) -> Vector3 {
			return Serializer::DeserializeVec3(&jsonObject);
		};
		return true;
	}
	template<>
	inline bool LuaAccessable::registerType<Vector4>() {
		auto& type = userType<Vector4>("Vec4",
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
		return true;
	}
	template<>
	inline bool LuaAccessable::registerType<Vector2>() {
		auto& type = userType<Vector2>("Vec2",
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
		return true;
	}
	template<>
	inline bool LuaAccessable::registerType<glm::quat>() {
		auto& type = userType<glm::quat>("Quat");
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
		return true;
	}

}
