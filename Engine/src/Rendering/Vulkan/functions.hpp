#pragma once
#include "common.hpp"
#include <vulkan/vulkan.h>
#include <map>

namespace NAME_SPACE {
	VkAttachmentDescription getResolveAttachment(VkFormat format);
	VkAttachmentDescription getDepthAttachment(VkFormat format, VkSampleCountFlagBits sampleCount);
	VkAttachmentDescription getColorAttachment(VkFormat format, VkSampleCountFlagBits sampleCount, VkImageLayout finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);



}