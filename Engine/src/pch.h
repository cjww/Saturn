#pragma once

#define DEBUG_LOG

// Standard lib
#include <vector>
#include <map>
#include <array>
#include <unordered_map>

#include <stdexcept>

#include <chrono>

#include <fstream>
#include <iostream>
#include <string>
#include <memory>
#include <filesystem>
#include <algorithm>
#include <sstream>
#include <string.h>

#include <functional>


// Dependencies
#include <entt/entt.hpp>
#include <rapidxml/rapidxml.hpp>

#define SOL_ALL_SAFTIES_ON 1
#include <sol/sol.hpp>
#include <lua.hpp>



//#include <taskflow/taskflow.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define NOMINMAX
#include <SPIRV\spirv_cross.hpp>




// glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/mat4x4.hpp>

#include <imgui.h>

// Other
#include "Graphics/Vulkan/Renderer.hpp"
#include "Graphics/RenderWindow.hpp"

#include <Tools/Logger.hpp>
#include <Tools/utils.h>




