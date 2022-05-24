#include "pch.h"
#include "debugFunctions.h"

namespace sa {
    
    void checkError(vk::Result result, const char* msg, bool doThrow) {
        std::stringstream ss;
        if (msg) ss << msg << " : ";

        switch (result) {
        case vk::Result::eErrorOutOfHostMemory:
            ss << "Out of host memory" << std::endl;
            break;
        case vk::Result::eErrorOutOfDeviceMemory:
            ss << "Out of device memory" << std::endl;
            break;
        case vk::Result::eErrorInitializationFailed:
            ss << "Initialization failed" << std::endl;
            break;
        case vk::Result::eErrorDeviceLost:
            ss << "Device Lost" << std::endl;
            break;
        case vk::Result::eErrorMemoryMapFailed:
            ss << "Memory map failed" << std::endl;
            break;
        case vk::Result::eErrorLayerNotPresent:
            ss << "Layer not present" << std::endl;
            break;
        case vk::Result::eErrorExtensionNotPresent:
            ss << "Extension not present" << std::endl;
            break;
        case vk::Result::eErrorFeatureNotPresent:
            ss << "Feature not present" << std::endl;
            break;
        case vk::Result::eErrorIncompatibleDriver:
            ss << "Incompatible drivers" << std::endl;
            break;
        case vk::Result::eErrorTooManyObjects:
            ss << "Too many objects" << std::endl;
            break;
        case vk::Result::eErrorFormatNotSupported:
            ss << "Format not supported" << std::endl;
            break;
        case vk::Result::eErrorFragmentedPool:
            ss << "Fragmented pool" << std::endl;
            break;
        case vk::Result::eErrorUnknown:
            ss << "Unknown" << std::endl;
            break;
        case vk::Result::eErrorOutOfPoolMemory:
            ss << "Out of pool memory" << std::endl;
            break;
        case vk::Result::eErrorInvalidExternalHandle:
            ss << "Invalid external handle" << std::endl;
            break;
        case vk::Result::eErrorFragmentation:
            ss << "Fragmentation" << std::endl;
            break;
        case vk::Result::eErrorInvalidOpaqueCaptureAddress:
            ss << "Invalid opaque capture adress" << std::endl;
            break;
        case vk::Result::eErrorSurfaceLostKHR:
            ss << "Surface lost" << std::endl;
            break;
        case vk::Result::eErrorNativeWindowInUseKHR:
            ss << "Native window in use KHR" << std::endl;
            break;
        case vk::Result::eErrorOutOfDateKHR:
            ss << "Out of date KHR" << std::endl;
            break;
        case vk::Result::eErrorIncompatibleDisplayKHR:
            ss << "Incompatible display KHR" << std::endl;
            break;
        case vk::Result::eErrorValidationFailedEXT:
            ss << "Validation failed EXT" << std::endl;
            break;
        case vk::Result::eErrorInvalidShaderNV:
            ss << "Invalid shader NV" << std::endl;
            break;
        case vk::Result::eErrorInvalidDrmFormatModifierPlaneLayoutEXT:
            ss << "Invalid DRM format modifier plane layout EXT" << std::endl;
            break;
        case vk::Result::eErrorNotPermittedEXT:
            ss << "Not permitted EXT" << std::endl;
            break;
        default:
            return;
        }
        if (doThrow) {
            throw std::runtime_error(ss.str());
        }
        else {
            DEBUG_LOG_ERROR(ss.str());
        }
    }

    void fillDebugMessengerInfo(VkDebugUtilsMessengerCreateInfoEXT& infso) {
    }

    VkResult setupDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT* debugMessenger) {

        VkDebugUtilsMessengerCreateInfoEXT msgInfo = {};
        fillDebugMessengerInfo(msgInfo);
        return createDebugUtilsMessengerEXT(instance, &msgInfo, nullptr, debugMessenger);
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
    {
        std::stringstream ss;
        ss << pCallbackData->pMessage << std::endl;

        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
            DEBUG_LOG_WARNING(ss.str());
        }
        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
            DEBUG_LOG_ERROR(ss.str());
        }
        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
            DEBUG_LOG_INFO(ss.str());
        }
        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
            ss << " VERBOSE ";
            DEBUG_LOG_INFO(ss.str());
        }
        return VK_FALSE;
    }

    VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
    {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        }
        else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(instance, debugMessenger, pAllocator);
        }
    }
}