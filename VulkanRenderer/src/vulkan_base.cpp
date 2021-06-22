#include "vulkan_base.hpp"

namespace vbl {

    VkResult getAvailableExtentions(const char** extensionNames, uint32_t* count) {
        VkResult res = vkEnumerateInstanceExtensionProperties(nullptr, count, nullptr);
        if(extensionNames == nullptr) {
            return res;
        }
        VkExtensionProperties* availableExtensions = new VkExtensionProperties[*count];
        res = vkEnumerateInstanceExtensionProperties(nullptr, count, availableExtensions);
        for(uint32_t i = 0; i < *count; i++){
            extensionNames[i] = availableExtensions[i].extensionName;
        }
		delete[] availableExtensions;
        return res;
    }

    int rankPhysicalDevice(VkPhysicalDevice physicalDevice) {
        VkPhysicalDeviceProperties properties;
        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceProperties(physicalDevice, &properties);
        vkGetPhysicalDeviceFeatures(physicalDevice, &features);

        int score = 0;
        score += properties.limits.maxColorAttachments;
        score += properties.limits.framebufferColorSampleCounts;
        if(properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU){
            score += 1000;
        }
        //score += properties.limits.maxImageDimension2D;
        
        if(!features.geometryShader){
            return 0;
        }
        return score;
    }

    uint32_t getQueueFamilyIndex(VkPhysicalDevice physicalDevice, VkQueueFlags capabilities, VkQueueFamilyProperties* prop) {
        uint32_t count;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, nullptr);
        VkQueueFamilyProperties* properties = new VkQueueFamilyProperties[count];
		
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, properties);

        for(uint32_t i = 0; i < count; i++) {
            if(properties[i].queueCount > 0) {
                if(properties[i].queueFlags & capabilities){
                    if(prop != nullptr) {
                        *prop = properties[i];
                    }
					delete[] properties;
                    return i;
                }
            }
        }
		delete[] properties;
        return -1;
    } 

	QueueInfo getQueueInfo(VkPhysicalDevice physicalDevice, VkQueueFlags capabilities, uint32_t queueCount) {
		VkQueueFamilyProperties prop;
		vbl::QueueInfo queueInfo = {};
		queueInfo.family = getQueueFamilyIndex(physicalDevice, capabilities, &prop);
		if (queueInfo.family == (uint32_t)-1) {
			throw std::runtime_error("Found no suitable queue family");
		}

		queueInfo.queueCount = vbl::clamp(queueCount, 1u, prop.queueCount);
		queueInfo.priorities.resize(queueInfo.queueCount);
		for (uint32_t i = 0; i < queueInfo.queueCount; i++) {
			queueInfo.priorities[i] = 1.0f;
		}
        
		return queueInfo;
	}

    uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typefilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
        
        for(uint32_t i = 0; i < memProperties.memoryTypeCount; i++){
            if((typefilter & (1 << i)) &&
                (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }
        throw std::runtime_error("Failed to find suitable memory");
    }

    VkResult createInstance(VkInstance* instance, const char* desiredExtensions[], uint32_t desiredExtensionCount, VkApplicationInfo* appInfo){
        VkInstanceCreateInfo instanceInfo = {};
        instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceInfo.flags = 0;
        instanceInfo.pApplicationInfo = appInfo;

        instanceInfo.ppEnabledExtensionNames = desiredExtensions;
        instanceInfo.enabledExtensionCount = desiredExtensionCount;
        std::cout << "---Extensions---" << std::endl;
        for(uint32_t i = 0; i < desiredExtensionCount; i++) {
            std::cout << desiredExtensions[i] << std::endl;
        }
        instanceInfo.ppEnabledLayerNames = nullptr;
        instanceInfo.enabledLayerCount = 0;
        instanceInfo.pNext = nullptr;
        VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo;
        if(vbl_useValidationLayers) {
            instanceInfo.ppEnabledLayerNames = vbl_validationLayers;
            instanceInfo.enabledLayerCount = vbl_validationLayerCount;
            std::cout << "---Layers---" << std::endl;
            for(uint32_t i = 0; i < vbl_validationLayerCount; i++) {
                std::cout << vbl_validationLayers[i] << std::endl;
            }

            fillDebugMessengerInfo(debugMessengerCreateInfo);
            instanceInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugMessengerCreateInfo;
        }
        return vkCreateInstance(&instanceInfo, nullptr, instance);
    }

    VkResult getPhysicalDevices(VkInstance instance, VkPhysicalDevice* devices, uint32_t amount) {
        uint32_t count;
        VkResult res = vkEnumeratePhysicalDevices(instance, &count, VK_NULL_HANDLE);
        if(res != VK_SUCCESS){
            return res;
        }
        VkPhysicalDevice* physicalDevices = new VkPhysicalDevice[count];
        res = vkEnumeratePhysicalDevices(instance, &count, physicalDevices);
        std::multimap<int, VkPhysicalDevice, std::greater<int>> map;
        for(uint32_t i = 0; i < count; i++) {
            int score = rankPhysicalDevice(physicalDevices[i]);
            map.insert(std::make_pair(score, physicalDevices[i]));
        }
        
		delete[] physicalDevices;

        uint32_t i = 0;
        for(const auto& device : map) {
            devices[i] = device.second;
            i++;
            if(i == amount){
                break;
            }
        }
        return res;
    }

    VkResult createLogicalDevice(VkDevice* device, VkPhysicalDevice physicalDevice, const char* desiredExtensions[], uint32_t desiredExtensionCount,
        QueueInfo* queueInfos, uint32_t queueInfoCount, VkPhysicalDeviceFeatures* desiredFeatures)
    {

        VkDeviceQueueCreateInfo* queueCreateInfos = new VkDeviceQueueCreateInfo[queueInfoCount];
        
        for(uint32_t i = 0; i < queueInfoCount; i++){
            queueCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfos[i].flags = 0;
            queueCreateInfos[i].pNext = nullptr;
            queueCreateInfos[i].queueFamilyIndex = queueInfos[i].family;
            queueCreateInfos[i].queueCount = static_cast<uint32_t>(queueInfos[i].queueCount);
            queueCreateInfos[i].pQueuePriorities = queueInfos[i].priorities.data();
        }
        VkDeviceCreateInfo deviceInfo = {};
        deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceInfo.pNext = nullptr;
        deviceInfo.pQueueCreateInfos =  queueCreateInfos;
        deviceInfo.queueCreateInfoCount = queueInfoCount;

        deviceInfo.ppEnabledExtensionNames = desiredExtensions;
        deviceInfo.enabledExtensionCount = desiredExtensionCount;
        
        deviceInfo.ppEnabledLayerNames = nullptr;
        deviceInfo.enabledLayerCount = 0;
        if(vbl_useValidationLayers){
            deviceInfo.ppEnabledLayerNames = vbl_validationLayers;
            deviceInfo.enabledLayerCount = vbl_validationLayerCount;
        }

        deviceInfo.pEnabledFeatures = desiredFeatures;

        VkResult res = vkCreateDevice(physicalDevice, &deviceInfo, nullptr, device);
		delete[] queueCreateInfos;
		return res;
    }

    VkResult createSwapchain(VkSwapchainKHR* swapchain, VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, QueueInfo* queueInfos, uint32_t queueInfoCount, VkFormat* finalImageFormat, VkExtent2D* imageSize) {
        VkResult res;
        for(uint32_t i = 0; i < queueInfoCount; i++){
            VkBool32 surfaceSupported = VK_FALSE;
            res = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueInfos[i].family, surface, &surfaceSupported);
            if(res != VK_SUCCESS){
                return res;
            }
            if(!surfaceSupported){
                std::cout << "All queues not supported" << std::endl;
                return VK_NOT_READY;
            }
        }
        
        VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
        res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);
        if(res != VK_SUCCESS){
            return res;
        }
        
        uint32_t minImageCount = surfaceCapabilities.minImageCount + 1;
        if(surfaceCapabilities.maxImageCount > 0 && minImageCount > surfaceCapabilities.maxImageCount){
            minImageCount = surfaceCapabilities.maxImageCount;
        }

        VkExtent2D finalImageSize = surfaceCapabilities.currentExtent;
        if(imageSize != nullptr){
            finalImageSize = *imageSize;
        }
        finalImageSize = clamp(finalImageSize, surfaceCapabilities.minImageExtent, surfaceCapabilities.maxImageExtent);

        uint32_t formatCount;
        res = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
        if(res != VK_SUCCESS || formatCount == 0){
            return res;
        }
        VkSurfaceFormatKHR* formats = new VkSurfaceFormatKHR[formatCount];
        res = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats);


        VkSurfaceFormatKHR finalFormat = formats[0];
        for(uint32_t i = 0; i < formatCount; i++) {
            VkFormatProperties properties;
            vkGetPhysicalDeviceFormatProperties(physicalDevice, formats[i].format, &properties);
            if(properties.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT){
                finalFormat = formats[i];
                break;
            }
        }
        *finalImageFormat = finalFormat.format;
		delete[] formats;

        uint32_t presentModeCount;
        res = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
        if(res != VK_SUCCESS || presentModeCount == 0){
            return res;
        }
        VkPresentModeKHR* presentModes = new VkPresentModeKHR[presentModeCount];
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes);
        
        VkPresentModeKHR presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
        bool found = false;
        for(uint32_t i = 0; i < presentModeCount && !found; i++) {
            if(presentModes[i] == presentMode){
                found = true;
            }
        }
		delete[] presentModes;
        if(!found){
            presentMode = VK_PRESENT_MODE_FIFO_KHR;
        }

        VkSwapchainCreateInfoKHR swapchainInfo = {};
        swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchainInfo.pNext = nullptr;
		swapchainInfo.flags = 0;
        swapchainInfo.clipped = VK_TRUE;
        swapchainInfo.minImageCount = minImageCount;
        swapchainInfo.imageExtent = finalImageSize;
        swapchainInfo.imageArrayLayers = 1;
        swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        swapchainInfo.imageColorSpace = finalFormat.colorSpace;
        swapchainInfo.imageFormat = finalFormat.format;
        swapchainInfo.oldSwapchain = *swapchain;
        uint32_t* indices = new uint32_t[queueInfoCount];
        for(uint32_t i = 0; i < queueInfoCount; i++){
            indices[i] = queueInfos[i].family;
        }
        swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainInfo.pQueueFamilyIndices = indices;
        swapchainInfo.queueFamilyIndexCount = queueInfoCount;
        swapchainInfo.surface = surface;
        swapchainInfo.preTransform = surfaceCapabilities.currentTransform;
        swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapchainInfo.presentMode = presentMode;
		res = vkCreateSwapchainKHR(device, &swapchainInfo, nullptr, swapchain);
		delete[] indices;
		return res;
    }

    VkResult createSemaphore(VkSemaphore* semaphore, VkDevice device, VkSemaphoreCreateFlags flags, void* pNext) {
        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphoreInfo.pNext = pNext;
        semaphoreInfo.flags = flags;
        return vkCreateSemaphore(device, &semaphoreInfo, nullptr, semaphore);
    }

    VkResult createSemaphores(VkSemaphore* semaphore, uint32_t count, VkDevice device, VkSemaphoreCreateFlags flags, void* pNext) {
        VkResult res = VK_SUCCESS;
        for(uint32_t i = 0; i < count; i++){
            res = createSemaphore(&semaphore[i], device, flags, pNext);
            if(res != VK_SUCCESS){
                return res;
            }
        }
        return res;
    }

    VkResult createFence(VkFence* fence, VkDevice device, VkFenceCreateFlags flags, void* pNext) {
        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.pNext = pNext;
        fenceInfo.flags = flags;
        return vkCreateFence(device, &fenceInfo, nullptr, fence);
    }

    VkResult createFences(VkFence* fences, uint32_t count, VkDevice device, VkFenceCreateFlags flags, void* pNext) {
        VkResult res;
        for(uint32_t i = 0; i < count; i++){
            res = createFence(&fences[i], device, flags, pNext);
            if(res != VK_SUCCESS){
                return res;
            }
        }
        return res;
    }

    VkResult presentImage(VkQueue queue, uint32_t imageIndex, VkSwapchainKHR swapchain, VkSemaphore waitSemaphore) {

        VkResult res;
        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.pWaitSemaphores = waitSemaphore == VK_NULL_HANDLE ? nullptr : &waitSemaphore;
        presentInfo.waitSemaphoreCount = waitSemaphore == VK_NULL_HANDLE ? 0 : 1;
        presentInfo.pSwapchains = &swapchain;
        presentInfo.swapchainCount = 1;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = &res;
        return vkQueuePresentKHR(queue, &presentInfo);
    } 
    
    VkResult presentImages(VkQueue queue, uint32_t count, uint32_t* imageIndices, VkSwapchainKHR* swapchains, VkSemaphore* waitSemaphores) {
		VkResult* res = nullptr;
		if (count > 1) {
			res = new VkResult[count];
		}
        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.pWaitSemaphores = waitSemaphores;
        presentInfo.waitSemaphoreCount = waitSemaphores == nullptr ? 0 : count;
        presentInfo.pSwapchains = swapchains;
        presentInfo.swapchainCount = count;
        presentInfo.pImageIndices = imageIndices;
        presentInfo.pResults = res;
        VkResult firstRes = vkQueuePresentKHR(queue, &presentInfo);
        for(uint32_t i = 0; i < count; i++) {
            if(res[i] != VK_SUCCESS){
				VkResult finalRes = res[i];
				delete[] res;
				return finalRes;
			}
        }
		if (res != nullptr) {
			delete[] res;
		}
        return firstRes;
    }

    VkResult createCommandPool(VkCommandPool* commandPool, VkDevice device, uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags) {
        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = queueFamilyIndex;
        poolInfo.flags = flags;
        return vkCreateCommandPool(device, &poolInfo, nullptr, commandPool);
    }

    VkResult allocateCommandBuffers(VkCommandBuffer* commandBuffers, uint32_t count, VkDevice device, VkCommandPool commandPool, VkCommandBufferLevel level) {
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = count;
        allocInfo.level = level;
        return vkAllocateCommandBuffers(device, &allocInfo, commandBuffers);
    }

    VkResult beginPrimaryCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferUsageFlags usage) {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.pInheritanceInfo = nullptr;
        beginInfo.flags = usage;
        return vkBeginCommandBuffer(commandBuffer, &beginInfo);
    }

    VkResult beginSecondaryCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferUsageFlags usage,
        VkFramebuffer framebuffer, VkRenderPass renderPass, uint32_t subpassIndex, 
        VkQueryPipelineStatisticFlags pipelineStatisticsFlags, VkQueryControlFlags queryControlFlags)
    {
        
        VkCommandBufferInheritanceInfo inheritInfo = {};
        inheritInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
        
        inheritInfo.framebuffer = framebuffer;
        
        inheritInfo.queryFlags = queryControlFlags;
        inheritInfo.occlusionQueryEnable = (VkBool32) queryControlFlags != 0;
        
        inheritInfo.pipelineStatistics = pipelineStatisticsFlags;

        inheritInfo.renderPass = renderPass;
        inheritInfo.subpass = subpassIndex;

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.pInheritanceInfo = &inheritInfo;
        beginInfo.flags = usage;
        return vkBeginCommandBuffer(commandBuffer, &beginInfo);
    }

    VkResult createRenderPass(VkRenderPass *renderPass, VkDevice device, VkAttachmentDescription* attachments, uint32_t attachmentCount, SubpassParameters* subpassParams, uint32_t subpassCount, const VkSubpassDependency* dependencies, uint32_t dependencyCount) {
        VkSubpassDescription* subpasses = new VkSubpassDescription[subpassCount];
        for(uint32_t i = 0; i < subpassCount; i++){
            VkSubpassDescription subpassDesc = {};
            subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpassDesc.colorAttachmentCount = subpassParams[i].colorAttachmentCount;
            subpassDesc.pColorAttachments = subpassParams[i].colorAttachments;
            subpassDesc.pResolveAttachments = subpassParams[i].resolveAttachments; 

            subpassDesc.inputAttachmentCount = subpassParams[i].inputAttachmentCount;
            subpassDesc.pInputAttachments = subpassParams[i].inputAttachments;
            
            subpassDesc.pDepthStencilAttachment = subpassParams[i].depthStencilAttachment;
            
            subpassDesc.preserveAttachmentCount = subpassParams[i].preserveAttachmentCount;
            subpassDesc.pPreserveAttachments = subpassParams[i].preserveAttachments;
            subpasses[i] = subpassDesc;
        }

        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = attachmentCount;
        renderPassInfo.pAttachments = attachments;
        renderPassInfo.subpassCount = subpassCount;
        renderPassInfo.pSubpasses = subpasses;
        renderPassInfo.dependencyCount = dependencyCount;
        renderPassInfo.pDependencies = dependencies;
        
		VkResult res = vkCreateRenderPass(device, &renderPassInfo, nullptr, renderPass);
		delete[] subpasses;
		return res;
    }

    VkResult createImageView2D(VkImageView* imageView, VkDevice device, VkImage image, VkFormat imageFormat, VkImageAspectFlags imageAspect) {
        VkImageViewCreateInfo imageViewInfo = {};
        imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewInfo.format = imageFormat;
        imageViewInfo.image = image;
        imageViewInfo.subresourceRange.aspectMask = imageAspect;
        imageViewInfo.subresourceRange.baseMipLevel = 0;
        imageViewInfo.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
        imageViewInfo.subresourceRange.baseArrayLayer = 0;
        imageViewInfo.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
        imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        return vkCreateImageView(device, &imageViewInfo, nullptr, imageView);
    }

    VkResult createFramebuffer(VkFramebuffer* framebuffer, VkDevice device, VkRenderPass renderPass, VkImageView* attachmentImageViews, uint32_t attachmentCount, VkExtent2D size, uint32_t layers) {
        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.attachmentCount = attachmentCount;
        framebufferInfo.pAttachments = attachmentImageViews;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.height = size.height;
        framebufferInfo.width = size.width;
        framebufferInfo.layers = layers;

        return vkCreateFramebuffer(device, &framebufferInfo, nullptr, framebuffer);
    }

    std::vector<char> readFile(const char* path) {
        std::ifstream file(path, std::ios::ate | std::ios::binary);
        if(!file.is_open()){
            throw std::runtime_error("Failed to open file " + std::string(path));
        }
        const size_t fileSize = file.tellg();
        file.seekg(0);
		std::vector<char> buffer(fileSize);
		file.read(buffer.data(), fileSize);
        file.close();

        return std::move(buffer);
    }

    VkResult createShaderModule(VkShaderModule* shader, VkDevice device, const char* sprivPath) {
        
		std::vector<char> buffer = readFile(sprivPath);

        VkShaderModuleCreateInfo shaderInfo = {};
        shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shaderInfo.codeSize = buffer.size();
        shaderInfo.pCode = (const uint32_t*)buffer.data();
        return vkCreateShaderModule(device, &shaderInfo, nullptr, shader);
    }

    VkResult createShaderModule(VkShaderModule* shader, VkDevice device, const uint32_t* code, size_t codeSize) {
        VkShaderModuleCreateInfo shaderInfo = {};
        shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shaderInfo.codeSize = codeSize;
        shaderInfo.pCode = code;
        return vkCreateShaderModule(device, &shaderInfo, nullptr, shader);
    }

    void fillPipelineShaderStageCreateInfo(VkPipelineShaderStageCreateInfo* info, VkShaderModule shaderModule, VkShaderStageFlagBits stage) {
        *info = {};
        info->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        info->module = shaderModule;
        info->pName = "main";
        info->stage = stage;
    }
    
    VkResult createDescriptorPool(VkDescriptorPool* descriptorPool, VkDevice device, VkDescriptorPoolSize* poolSizes, uint32_t poolSizeCount, uint32_t maxSets) {
		VkDescriptorPoolCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		info.poolSizeCount = poolSizeCount;
		info.pPoolSizes = poolSizes;
		info.maxSets = maxSets;
		return vkCreateDescriptorPool(device, &info, nullptr, descriptorPool);
	}

    VkResult createDescriptorSetLayout(VkDescriptorSetLayout* descriptorSetLayout, VkDevice device, VkDescriptorSetLayoutBinding* setBindings, uint32_t setBindingCount) {

        VkDescriptorSetLayoutCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        info.bindingCount = setBindingCount;
        info.pBindings = setBindings;
        return vkCreateDescriptorSetLayout(device, &info, nullptr, descriptorSetLayout);
    }

    VkResult allocateDescriptorSets(VkDescriptorSet* descriptorSets, VkDevice device, VkDescriptorSetLayout* descriptorSetLayouts, uint32_t descriptorSetCount, VkDescriptorPool descriptorPool) {

        VkDescriptorSetAllocateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        info.descriptorPool = descriptorPool;
        info.descriptorSetCount = descriptorSetCount;
        info.pSetLayouts = descriptorSetLayouts;
        return vkAllocateDescriptorSets(device, &info, descriptorSets);
    }

    VkResult freeDesciptorSets(VkDevice device, VkDescriptorSet* descriptorSets, uint32_t descriptorSetCount, VkDescriptorPool descriptorPool) {
        return vkFreeDescriptorSets(device, descriptorPool, descriptorSetCount, descriptorSets);
    }

    VkResult createPipelineLayout(VkPipelineLayout* pipelineLayout, VkDevice device, const VkDescriptorSetLayout* descriptorSetLayouts, uint32_t descriptorSetLayoutCount, const VkPushConstantRange* pushConstantRanges, uint32_t pushConstantRangeCount) {
        VkPipelineLayoutCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        info.setLayoutCount = descriptorSetLayoutCount;
        info.pSetLayouts = descriptorSetLayouts;
        info.pushConstantRangeCount = pushConstantRangeCount;
        info.pPushConstantRanges = pushConstantRanges;
        
        return vkCreatePipelineLayout(device, &info, nullptr, pipelineLayout);
    }

	VkResult createGraphicsPipeline(VkPipeline* pipeline, VkDevice device, VkPipelineLayout layout, VkRenderPass renderPass, uint32_t subpassIndex, VkExtent2D extent, const VkPipelineShaderStageCreateInfo* shaderStages, uint32_t shaderStageCount, VkPipelineVertexInputStateCreateInfo vertexInput, const VkDynamicState* pDynamicStates, uint32_t dynamicStateCount) {

        vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInput.pNext = nullptr;
        vertexInput.flags = 0;

        VkPipelineInputAssemblyStateCreateInfo input = {};
        input.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input.primitiveRestartEnable = VK_FALSE;
        input.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        VkPipelineRasterizationStateCreateInfo rasterizer = {};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.lineWidth = 1.0f;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisample = {};
        multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisample.sampleShadingEnable = VK_FALSE;
        multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineColorBlendAttachmentState colorAttachment = {};
		colorAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorAttachment.blendEnable = VK_FALSE;
		colorAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
        colorAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        colorAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;

		VkPipelineColorBlendStateCreateInfo colorBlend = {};
	    colorBlend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlend.attachmentCount = 1;
        colorBlend.pAttachments = &colorAttachment;
        colorBlend.logicOpEnable = VK_FALSE;


        VkPipelineDepthStencilStateCreateInfo depthState = {};
        depthState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthState.depthBoundsTestEnable = VK_FALSE;
        depthState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        depthState.depthTestEnable = VK_TRUE;
        depthState.depthWriteEnable = VK_TRUE;
        depthState.stencilTestEnable = VK_FALSE;

        VkViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = extent.width;
        viewport.height = extent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor = {};
        scissor.extent = extent;
        scissor.offset = {0, 0};

        VkPipelineViewportStateCreateInfo viewportState = {};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;

        VkPipelineDynamicStateCreateInfo dynamicState = {};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.pNext = nullptr;
        dynamicState.flags = 0;
        dynamicState.pDynamicStates = pDynamicStates;
        dynamicState.dynamicStateCount = dynamicStateCount;

		VkGraphicsPipelineCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		info.layout = layout;
		info.renderPass = renderPass;
        info.pColorBlendState = &colorBlend;
        info.pDepthStencilState = &depthState;
        info.pDynamicState = &dynamicState;
        info.pInputAssemblyState = &input;
        info.pMultisampleState = &multisample;
        info.pRasterizationState = &rasterizer;
        info.stageCount = shaderStageCount;
        info.pStages = shaderStages;
        info.pTessellationState = nullptr;
        info.pVertexInputState = &vertexInput;
		info.subpass = subpassIndex;
        info.pViewportState = &viewportState;

		return vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &info, nullptr, pipeline);
	}

	VkResult createRaytracingPipeline(VkInstance instance, VkPipeline* pipeline, VkDevice device) {
		VkRayTracingShaderGroupCreateInfoNV shaderGroup = {};
        shaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;

        VkPipelineShaderStageCreateInfo shaderStage = {};
        
        
        VkRayTracingPipelineCreateInfoNV info = {};
        info.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV;
        info.basePipelineHandle = VK_NULL_HANDLE;
        
        info.flags = 0;
        return vkCreateRayTracingPipelines(instance, device, VK_NULL_HANDLE, 1, &info, nullptr, pipeline);

	}

    VkResult vkCreateRayTracingPipelines(VkInstance instance, VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoNV *pCreateInfos, const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines) {
        PFN_vkCreateRayTracingPipelinesNV func = (PFN_vkCreateRayTracingPipelinesNV) vkGetInstanceProcAddr(instance, "vkCreateRayTracingPipelinesNV");
        if(func){
            return func(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
        }
        else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    VkResult createComputePipeline(VkPipeline* pipeline, VkDevice device, VkPipelineLayout layout, VkPipelineShaderStageCreateInfo shaderStage) {
        VkPipelineCreateFlags options = 0;
        VkComputePipelineCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        info.basePipelineHandle = VK_NULL_HANDLE;
        info.basePipelineIndex = -1;
        info.flags = options;
        info.layout = layout;
        info.pNext = nullptr;
        info.stage = shaderStage;
        
        return vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &info, nullptr, pipeline);
    }

    VkResult createImage(VkImage* image, VkDevice device, VkExtent3D extent, VkImageUsageFlags usage, VkFormat format, VkImageTiling tiling, uint32_t* queueFamilies, uint32_t queueFamilyCount) {

        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.arrayLayers = 1;
        imageInfo.extent = extent;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.format = format;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.mipLevels = 1;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.tiling = tiling;
        imageInfo.usage = usage;
        imageInfo.pQueueFamilyIndices = queueFamilies;
        imageInfo.queueFamilyIndexCount = queueFamilyCount;
        return vkCreateImage(device, &imageInfo, nullptr, image);
    }

    VkResult createBuffer(VkBuffer* buffer, VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage, uint32_t* queueFamilies, uint32_t queueFamilyCount) {
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.queueFamilyIndexCount = queueFamilyCount;
        bufferInfo.pQueueFamilyIndices = queueFamilies;
        return vkCreateBuffer(device, &bufferInfo, nullptr, buffer);
    }

    VkResult createSampler(VkSampler* sampler, VkDevice device, VkFilter minMagFilter, float maxAnisotropy, float minLod, float maxLod, float mipLodBias) {
        VkSamplerCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        info.magFilter = minMagFilter;
        info.minFilter = minMagFilter;
        info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        info.anisotropyEnable = VK_FALSE;
        info.maxAnisotropy = 1.0f;
        if (maxAnisotropy > 1.0f) {
            info.anisotropyEnable = VK_TRUE;
            info.maxAnisotropy = maxAnisotropy;
        }
        info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        info.unnormalizedCoordinates = VK_FALSE;

        info.compareEnable = VK_FALSE;
        info.compareOp = VK_COMPARE_OP_ALWAYS;
        
        info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        info.mipLodBias = mipLodBias;
        info.minLod = minLod;
        info.maxLod = maxLod;
        return vkCreateSampler(device, &info, nullptr, sampler);

    }

    VkResult bindImageMemory(VkDeviceMemory* memory, VkDevice device, VkImage image, VkPhysicalDevice physicalDevice, VkMemoryPropertyFlags memoryFlags) {
        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(device, image, &memRequirements);

        VkMemoryAllocateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        info.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, memoryFlags);
        info.allocationSize = memRequirements.size;
        VkResult res = vkAllocateMemory(device, &info, nullptr, memory);
        if(res != VK_SUCCESS){
            return res;
        }

        res = vkBindImageMemory(device, image, *memory, 0);
        return res;
    }

    VkResult allocateBufferMemory(VkDeviceMemory* memory, VkDevice device, VkBuffer buffer, VkDeviceSize size, VkPhysicalDevice physicalDevice, VkMemoryPropertyFlags memoryFlags) {
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

        VkMemoryAllocateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        info.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, memoryFlags);
        std::cout << memRequirements.memoryTypeBits  << " + " << memoryFlags << ": " << info.memoryTypeIndex << std::endl;
        info.allocationSize = max(memRequirements.size, size);
        VkResult res = vkAllocateMemory(device, &info, nullptr, memory);
        return res;
    }

    VkResult bindBufferMemory(VkDeviceMemory memory, VkDevice device, VkBuffer buffer, VkDeviceSize offset) {
        return vkBindBufferMemory(device, buffer, memory, offset);
    }
    
    void flushMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize size, VkDeviceSize offset) {
        VkMappedMemoryRange memRange = {};
        memRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        memRange.memory = memory;
        memRange.offset = offset;
        memRange.size = size;
        vkFlushMappedMemoryRanges(device, 1, &memRange);
        vkInvalidateMappedMemoryRanges(device, 1, &memRange);
    }

    void cmdTransitionImages(VkCommandBuffer commandBuffer, ImageTransition* transitions, uint32_t count, VkPipelineStageFlags generatingStages, VkPipelineStageFlags consumingStages) {
        VkImageMemoryBarrier* barriers = new VkImageMemoryBarrier[count];
        for(uint32_t i = 0; i < count; i++){
            barriers[i].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barriers[i].pNext = nullptr;
            barriers[i].image = transitions[i].image;
            
            barriers[i].srcAccessMask = transitions[i].currentAccess;
            barriers[i].dstAccessMask = transitions[i].newAccess;
            barriers[i].oldLayout = transitions[i].currentImageLayout;
            barriers[i].newLayout = transitions[i].newImageLayout;
            barriers[i].srcQueueFamilyIndex = transitions[i].currentQueueFamily;
            barriers[i].dstQueueFamilyIndex = transitions[i].newQueueFamily;

            barriers[i].subresourceRange.aspectMask = transitions[i].aspect;
            barriers[i].subresourceRange.baseMipLevel = 0;
            barriers[i].subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
            barriers[i].subresourceRange.baseArrayLayer = 0;
            barriers[i].subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
        }

        vkCmdPipelineBarrier(commandBuffer, generatingStages, consumingStages, 0, 0, nullptr, 0, nullptr, count, barriers);
		delete[] barriers;
    }

    void printError(VkResult result, const char* msg, bool doThrow, FILE* output) {
        std::stringstream ss;
        ss << "Error: ";
        switch (result) {
        case VK_NOT_READY:
            ss << "VK_NOT_READY";
            break;
        case VK_TIMEOUT:
            ss << "VK_TIMEOUT";
            break;
        case VK_EVENT_SET:
            ss << "VK_EVENT_SET";
            break;
        case VK_EVENT_RESET:
            ss << "VK_EVENT_RESET";
            break;
        case VK_INCOMPLETE:
            ss << "VK_INCOMPLETE";
            break;
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            ss << "VK_ERROR_OUT_OF_HOST_MEMORY";
            break;
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            ss << "VK_ERROR_OUT_OF_DEVICE_MEMORY";
            break;
        case VK_ERROR_INITIALIZATION_FAILED:
            ss << "VK_ERROR_INITIALIZATION_FAILED";
            break;
        case VK_ERROR_DEVICE_LOST:
            ss << "VK_ERROR_DEVICE_LOST";
            break;
        case VK_ERROR_MEMORY_MAP_FAILED:
            ss << "VK_ERROR_MEMORY_MAP_FAILED";
            break;
        case VK_ERROR_LAYER_NOT_PRESENT:
            ss << "VK_ERROR_LAYER_NOT_PRESENT";
            break;
        case VK_ERROR_EXTENSION_NOT_PRESENT:
            ss << "VK_ERROR_EXTENSION_NOT_PRESENT";
            break;
        case VK_ERROR_FEATURE_NOT_PRESENT:
            ss << "VK_ERROR_FEATURE_NOT_PRESENT";
            break;
        case VK_ERROR_INCOMPATIBLE_DRIVER:
            ss << "VK_ERROR_INCOMPATIBLE_DRIVER";
            break;
        case VK_ERROR_TOO_MANY_OBJECTS:
            ss << "VK_ERROR_TOO_MANY_OBJECTS";
            break;
        case VK_ERROR_FORMAT_NOT_SUPPORTED:
            ss << "VK_ERROR_FORMAT_NOT_SUPPORTED";
            break;
        case VK_ERROR_FRAGMENTED_POOL:
            ss << "VK_ERROR_FRAGMENTED_POOL";
            break;
        case VK_ERROR_OUT_OF_POOL_MEMORY:
            ss << "VK_ERROR_OUT_OF_POOL_MEMORY";
            break;
        case VK_ERROR_INVALID_EXTERNAL_HANDLE:
            ss << "VK_ERROR_INVALID_EXTERNAL_HANDLE";
            break;
        case VK_ERROR_SURFACE_LOST_KHR:
            ss << "VK_ERROR_SURFACE_LOST_KHR";
            break;
        case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
            ss << "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
            break;
        case VK_SUBOPTIMAL_KHR:
            ss << "VK_SUBOPTIMAL_KHR";
            break;
        case VK_ERROR_OUT_OF_DATE_KHR:
            ss << "VK_ERROR_OUT_OF_DATE_KHR";
            break;
        case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
            ss << "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
            break;
        case VK_ERROR_VALIDATION_FAILED_EXT:
            ss << "VK_ERROR_VALIDATION_FAILED_EXT";
            break;
        case VK_ERROR_INVALID_SHADER_NV:
            ss << "VK_ERROR_INVALID_SHADER_NV";
            break;
        case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
            ss << "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
            break;
        case VK_ERROR_FRAGMENTATION_EXT:
            ss << "VK_ERROR_FRAGMENTATION_EXT";
            break;
        case VK_ERROR_NOT_PERMITTED_EXT:
            ss << "VK_ERROR_NOT_PERMITTED_EXT";
            break;
        case VK_ERROR_INVALID_DEVICE_ADDRESS_EXT:
            ss << "VK_ERROR_INVALID_DEVICE_ADDRESS_EXT";
            break;
        case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
            ss << "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
            break;
        default:

            break;
        }
        ss << " : ";
        if(msg != nullptr){
            ss << msg;
        }
        if(result != VK_SUCCESS){
            fprintf(output, "%s\n", ss.str().c_str());
            if(doThrow){
                throw std::runtime_error(ss.str());
            }
            
        }
    }

    void setValidationLayers(bool value, const char* layers[], uint32_t layerCount) {
        vbl_useValidationLayers = value;
        vbl_validationLayerCount = layerCount;
        vbl_validationLayers = layers;
    }

    void fillDebugMessengerInfo(VkDebugUtilsMessengerCreateInfoEXT& info) {
        info = {};
        info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        info.pNext = nullptr;
        info.pUserData = nullptr;
        info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        info.pfnUserCallback = debugCallback;
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
        if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT){
            ss << "---WARNING---" << std::endl;
        }
        if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT){
            ss << "----ERROR----" << std::endl;
        }
        if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT){
            ss << "----INFO----" << std::endl;
        }
        if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT){
            ss << "----VERBOSE----" << std::endl;
        }

        ss << pCallbackData->pMessage << std::endl;
        
        /*
        ss << "Object Count: " << pCallbackData->objectCount << std::endl;
        for(uint32_t i = 0; i < pCallbackData->objectCount; i++) {
            ss << pCallbackData->pObjects[i].pObjectName << std::endl;
        }
        */
        std::cerr << ss.str() << std::endl;
        return VK_FALSE;
    }

    VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) 
    {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        } else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(instance, debugMessenger, pAllocator);
        }
    }

} //!namespace vb