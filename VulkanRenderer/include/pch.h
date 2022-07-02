#pragma once

#define RENDERER_VALIDATION
#define DEBUG_LOG


#include <vector>
#include <unordered_map>
#include <queue>
#include <set>
#include <stdexcept>
#include <memory>
#include <functional>
#include <optional>
#include <fstream>
#include <chrono>

//#define VK_USE_PLATFORM_WIN32_KHR
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan\vulkan.hpp>

#include <GLFW/glfw3.h>


#include "Resources\ResourceManager.hpp"
#include "Tools\Logger.hpp"
#include "structs.hpp"
