#pragma once


// Standard lib
#include <vector>
#include <map>
#include <array>
#include <unordered_map>

#include <stdexcept>

#include <chrono>

#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <memory>
#include <filesystem>
#include <algorithm>
#include <sstream>
#include <string.h>

#include <functional>

#include <mutex>

// Dependencies
// entt - ECS library
#include <entt/entt.hpp>

// sol - Lua API
#define SOL_ALL_SAFTIES_ON 1
#include <sol/sol.hpp>
#include <lua.hpp>

// PhysX - Physics Engine
#include <PxPhysicsAPI.h>
#include <PxScene.h>
#include <PxRigidBody.h>


// simdjson - JSON parser
#include <simdjson/simdjson.h>

// glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/mat4x4.hpp>
#include <glm\geometric.hpp>

#include <glm/common.hpp>

// My code
#include "UUID.h"
//#include "Tools/Profiler.h"
//#include <Tools/Logger.hpp>
