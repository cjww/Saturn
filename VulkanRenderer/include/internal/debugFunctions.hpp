#pragma once

#include <vulkan\vulkan.hpp>
#include "Tools\Logger.hpp"

namespace sa {

    void checkError(vk::Result result, const std::string_view msg, bool doThrow = true);

    VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity, vk::Flags<vk::DebugUtilsMessageTypeFlagBitsEXT> messageType,
        const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
    /*
    VkResult setupDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT* debugMessenger);


    VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
    void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
    */

}