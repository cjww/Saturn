#pragma once

#define RENDERER_VALIDATION 1
#define DEBUG_LOG

#include <vector>
#include <unordered_map>
#include <queue>
#include <set>
#include <memory>
#include <functional>
#include <optional>
#include <fstream>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan\vulkan.hpp>

#include <GLFW/glfw3.h>

#define NOMINMAX
#include <SPIRV\spirv_cross.hpp>



#include "GLFW/glfw3.h"
#include "SPIRV\spirv.h"


#include "Resources\ResourceManager.hpp"
