#include "pch.h"
#include "debugFunctions.h"

namespace sa {
    
    void checkError(vk::Result result, const char* msg, bool doThrow) {
        if (result == vk::Result::eSuccess)
            return;
        std::stringstream ss;
        if (msg) 
            ss << msg << " : ";
        ss << vk::to_string(result);
        if (doThrow) {
            throw std::runtime_error(ss.str());
        }
        else {
            DEBUG_LOG_ERROR(ss.str());
        }
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

    /*
    VkResult setupDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT* debugMessenger) {

        VkDebugUtilsMessengerCreateInfoEXT msgInfo = {};
        fillDebugMessengerInfo(msgInfo);
        return createDebugUtilsMessengerEXT(instance, &msgInfo, nullptr, debugMessenger);
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
    */
}