#pragma once

#include <vulkan/vulkan.hpp>

inline bool vbl_useValidationLayers = false;
inline const char** vbl_validationLayers = nullptr;
inline uint32_t vbl_validationLayerCount = 0;
namespace vbl {

    //Structs
    struct QueueInfo {
      uint32_t family = 0;
      std::vector<float> priorities; 
      uint32_t queueCount;
    };

    struct ImageTransition {
        VkImage image;
        VkImageAspectFlags aspect;
        VkAccessFlags currentAccess;
        VkAccessFlags newAccess;
        VkImageLayout currentImageLayout;
        VkImageLayout newImageLayout;
        uint32_t currentQueueFamily = VK_QUEUE_FAMILY_IGNORED;
        uint32_t newQueueFamily = VK_QUEUE_FAMILY_IGNORED;
    };

    struct SubpassParameters {
        VkAttachmentReference* colorAttachments = nullptr;
        uint32_t colorAttachmentCount = 0;
        VkAttachmentReference* resolveAttachments = nullptr;

        VkAttachmentReference* inputAttachments = nullptr;
        uint32_t inputAttachmentCount = 0;
        
        const VkAttachmentReference* depthStencilAttachment = nullptr;
        
        uint32_t* preserveAttachments = nullptr;
        uint32_t preserveAttachmentCount = 0;
    };

    struct PipelineConfig {
        struct InputAssembly {
            VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        } input;
        
        struct Rasterizer {
            VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT;
            VkFrontFace frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
            VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
        } rasterizer;
        
        struct Multisample {
            VkBool32 enable = VK_FALSE;
            VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT;
        } multisample;
        struct ColorBlend {
            VkBool32 enable = VK_FALSE;
            VkBlendOp colorBlendOp = VK_BLEND_OP_ADD;
            VkBlendOp alphaBlendOp = VK_BLEND_OP_ADD;
            VkBlendFactor srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            VkBlendFactor dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            VkBlendFactor srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            VkBlendFactor dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        } colorBlend;
        struct DepthStencilState {
            VkBool32 depthBoundsTestEnable = VK_FALSE;
            VkCompareOp depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
            VkBool32 depthTestEnable = VK_TRUE;
            VkBool32 depthWriteEnable = VK_TRUE;
            VkBool32 stencilTestEnable = VK_FALSE;
        } depthStencil;
        
        std::vector<VkDynamicState> dynamicStates;

    };

    //Helpers
    template<typename T>
    inline T clamp(T value, T min, T max) {
        if(value < min) return min;
        else if(value > max) return max;
        return value;
    }

    template<>
    inline VkExtent2D clamp<VkExtent2D>(VkExtent2D value, VkExtent2D min, VkExtent2D max) {
        if(value.width < min.width) value.width = min.width;
        else if(value.width > max.width) value.width = max.width;
        if(value.height < min.height) value.height = min.height;
        else if(value.height > max.height) value.height = max.height;
        return value;
    }

    VkResult getAvailableExtentions(const char** extensionNames, uint32_t* count);
    int rankPhysicalDevice(VkPhysicalDevice physicalDevice);
    uint32_t getQueueFamilyIndex(VkPhysicalDevice physicalDevice, VkQueueFlags capabilities, VkQueueFamilyProperties* prop = nullptr);

	QueueInfo getQueueInfo(VkPhysicalDevice physicalDevice, VkQueueFlags capabilities, uint32_t queueCount);

    uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typefilter, VkMemoryPropertyFlags properties);

    //Main
    VkResult createInstance(VkInstance* instance, const char* desiredExtensions[], uint32_t extensionCount, VkApplicationInfo* appInfo = nullptr);

    VkResult getPhysicalDevices(VkInstance instance, VkPhysicalDevice* devices, uint32_t amount);

    VkResult createLogicalDevice(VkDevice* device, VkPhysicalDevice physicalDevice, const char* desiredExtensions[], uint32_t desiredExtensionCount,
        QueueInfo* queueInfos, uint32_t queueInfoCount, VkPhysicalDeviceFeatures* desiredFeatures = nullptr);

    VkResult createSwapchain(VkSwapchainKHR* swapchain, VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, QueueInfo* queueInfos, uint32_t queueInfoCount, VkFormat* finalImageFormat, VkExtent2D* imageSize = nullptr);
    VkResult recreateSwapchain(VkSwapchainKHR* swapchain, VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, QueueInfo* queueInfos, uint32_t queueInfoCount, VkFormat* finalImageFormat, VkExtent2D* imageSize = nullptr);

    VkResult createSemaphore(VkSemaphore* semaphore, VkDevice device, VkSemaphoreCreateFlags flags = 0, void* pNext = nullptr);
    VkResult createSemaphores(VkSemaphore* semaphore, uint32_t count, VkDevice device, VkSemaphoreCreateFlags flags = 0, void* pNext = nullptr);

    VkResult createFence(VkFence* fence, VkDevice device, VkFenceCreateFlags flags = 0, void* pNext = nullptr);
    VkResult createFences(VkFence* fences, uint32_t count, VkDevice device, VkFenceCreateFlags flags = 0, void* pNext = nullptr);

    VkResult presentImage(VkQueue queue, uint32_t imageIndex, VkSwapchainKHR swapchain, VkSemaphore waitSemaphore = VK_NULL_HANDLE);
    VkResult presentImages(VkQueue queue, uint32_t count, uint32_t* imageIndices, VkSwapchainKHR* swapchains, VkSemaphore* waitSemaphores = nullptr);
    
    VkResult createCommandPool(VkCommandPool* commandPool, VkDevice device, uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0);
    VkResult allocateCommandBuffers(VkCommandBuffer* commandBuffers, uint32_t count, VkDevice device, VkCommandPool commandPool, VkCommandBufferLevel level);

    VkResult beginPrimaryCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferUsageFlags usage);
    VkResult beginSecondaryCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferUsageFlags usage,
        VkFramebuffer framebuffer, VkRenderPass renderPass, uint32_t subpassIndex = 0,
        VkQueryPipelineStatisticFlags pipelineStatisticsFlags = 0, VkQueryControlFlags queryControlFlags = 0);

    VkResult createRenderPass(VkRenderPass *renderPass, VkDevice device, VkAttachmentDescription* attachments, uint32_t attachmentCount, SubpassParameters* subpassParams, uint32_t subpassCount, const VkSubpassDependency* dependencies, uint32_t dependencyCount);
    //createImage2D
    //VkResult createImage2D();
    VkResult createImageView2D(VkImageView* imageView, VkDevice device, VkImage image, VkFormat imageFormat, VkImageAspectFlags imageAspect);
    VkResult createFramebuffer(VkFramebuffer* framebuffer, VkDevice device, VkRenderPass renderPass, VkImageView* attachmentImageViews, uint32_t attachmentCount, VkExtent2D size, uint32_t layers = 1);
    
    std::vector<char> readFile(const char* path);
    
    VkResult createShaderModule(VkShaderModule* shader, VkDevice device, const char* sprivPath);
    VkResult createShaderModule(VkShaderModule* shader, VkDevice device, const uint32_t* code, size_t codeSize);
    void fillPipelineShaderStageCreateInfo(VkPipelineShaderStageCreateInfo* info, VkShaderModule shaderModule, VkShaderStageFlagBits stage);

	VkResult createDescriptorPool(VkDescriptorPool* descriptorPool, VkDevice device, VkDescriptorPoolSize* poolSizes, uint32_t poolSizeCount, uint32_t maxSets);
    VkResult createDescriptorSetLayout(VkDescriptorSetLayout* descriptorSetLayout, VkDevice device, VkDescriptorSetLayoutBinding* setBindings, uint32_t setBindingCount);
	VkResult allocateDescriptorSets(VkDescriptorSet* descriptorSets, VkDevice device, VkDescriptorSetLayout* descriptorSetLayouts, uint32_t descriptorSetCount, VkDescriptorPool descriptorPool);
    VkResult freeDesciptorSets(VkDevice device, VkDescriptorSet* descriptorSets, uint32_t descriptorSetCount, VkDescriptorPool descriptorPool);

    VkResult createPipelineLayout(VkPipelineLayout* pipelineLayout, VkDevice device, const VkDescriptorSetLayout* descriptorSetLayouts, uint32_t descriptorSetLayoutCount, const VkPushConstantRange* pushConstantRanges, uint32_t pushConstantRangeCount);

    VkResult createGraphicsPipeline(VkPipeline* pipeline, VkDevice device, VkPipelineLayout layout, VkRenderPass renderPass, uint32_t subpassIndex, VkExtent2D extent, const VkPipelineShaderStageCreateInfo* shaderStages, uint32_t shaderStageCount, VkPipelineVertexInputStateCreateInfo vertexInput, PipelineConfig config = {});
    
    VkResult createRayTracingPipeline(VkPipeline* pipeline, VkDevice device);
    VkResult vkCreateRayTracingPipelines(VkInstance instance, VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoNV *pCreateInfos, const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines);
    //VkResult vkDestroyRayTracingPipeline(VkInstance instance, VkDevice device, VkPipeline pipelconst VkAllocationCallbacks *pAllocator);
    
    VkResult createComputePipeline(VkPipeline* pipeline, VkDevice device, VkPipelineLayout layout, VkPipelineShaderStageCreateInfo shaderStage);
    
    VkResult createImage(VkImage* image, VkDevice device, VkExtent3D extent, VkImageUsageFlags usage, VkFormat format, VkImageTiling tiling, uint32_t* queueFamilies, uint32_t queueFamilyCount);
    VkResult createBuffer(VkBuffer* buffer, VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage, uint32_t* queueFamilies, uint32_t queueFamilyCount);

    VkResult createSampler(VkSampler* sampler, VkDevice device, VkFilter minMagFilter, float maxAnisotropy = 16.0f, float minLod = 0.0f, float maxLod = 0.0f, float mipLodBias = 0.0f);

    VkResult bindImageMemory(VkDeviceMemory* memory, VkDevice device, VkImage image, VkPhysicalDevice physicalDevice, VkMemoryPropertyFlags memoryFlags);

    VkResult allocateBufferMemory(VkDeviceMemory* memory, VkDevice device, VkBuffer buffer, VkDeviceSize size, VkPhysicalDevice physicalDevice, VkMemoryPropertyFlags memoryFlags);
    VkResult bindBufferMemory(VkDeviceMemory memory, VkDevice device, VkBuffer buffer, VkDeviceSize offset);

    void flushMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize size, VkDeviceSize offset);

    //transitionImage
    void cmdTransitionImages(VkCommandBuffer commandBuffer, ImageTransition* transitions, uint32_t count, VkPipelineStageFlags generatingStages, VkPipelineStageFlags consumingStages);

    //Debug
    void printError(VkResult result, const char* msg = nullptr, bool doThrow = false, FILE* output = stdout);

    void setValidationLayers(bool value, const char* layers[], uint32_t layerCount);

    void fillDebugMessengerInfo(VkDebugUtilsMessengerCreateInfoEXT& info);
    VkResult setupDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT* debugMessenger);
    VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

    VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
    void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

} // namespace vb