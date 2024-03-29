#pragma once

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
#if !defined(WIN32) and defined( _WIN32 )
#define WIN32
#endif



#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan\vulkan.hpp>

#include <GLFW/glfw3.h>


#include "Resources\ResourceManager.hpp"
#include "structs.hpp"
#include "Tools/Logger.hpp"
