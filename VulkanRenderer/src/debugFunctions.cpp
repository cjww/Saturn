#include "pch.h"
#include "internal/debugFunctions.hpp"

namespace sa {

    void checkError(vk::Result result, const std::string_view msg, bool doThrow) {
        if (result == vk::Result::eSuccess)
            return;

        std::stringstream ss;
        if (!msg.empty()) 
            ss << msg << " : ";
        ss << vk::to_string(result);
        if (doThrow) {
            throw std::runtime_error(ss.str());
        }
        else {
            SA_DEBUG_LOG_ERROR(ss.str());
        }
    }

    VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity, vk::Flags<vk::DebugUtilsMessageTypeFlagBitsEXT> messageType,
        const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
    {
        std::stringstream ss;
        ss << pCallbackData->pMessage << std::endl;

        if (messageSeverity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning) {
            SA_DEBUG_LOG_WARNING(ss.str());
        }
        if (messageSeverity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eError) {
            SA_DEBUG_LOG_ERROR(ss.str());
        }
        if (messageSeverity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo) {
            SA_DEBUG_LOG_INFO(ss.str());
        }
        if (messageSeverity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose) {
            ss << " VERBOSE ";
            SA_DEBUG_LOG_INFO(ss.str());
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