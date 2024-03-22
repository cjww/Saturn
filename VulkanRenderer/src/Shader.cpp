#include "pch.h"
#include "Shader.hpp"
#include "internal/VulkanCore.hpp"
#include "internal/DescriptorSet.hpp"

#include "vulkan\vulkan.hpp"

#include "Renderer.hpp"



#include <filesystem>

namespace sa {
	Shader::Shader() :
		m_pCore(Renderer::get().getCore())
	{

	}
	void Shader::create(const std::vector<uint32_t>& shaderCode) {
		vk::ShaderCreateInfoEXT shaderInfo = {};

		//spirv_cross::Compiler compiler(shaderCode);
		//const auto& entryPoints = compiler.get_entry_points_and_stages();
		//ShaderStageFlagBits stage = ShaderStageFlagBits(1U << (uint32_t)entryPoints[0].execution_model);

		/*

		ShaderStageInfo stageInfo = {};
		stageInfo.pCode = (uint32_t*)code.data();
		stageInfo.codeLength = code.size();
		stageInfo.pName = (char*)entryPoints[0].name.c_str();
		stageInfo.stage = stage;
		*/
		
		shaderInfo.codeSize = shaderCode.size() * sizeof(uint32_t);
		shaderInfo.pCode = shaderCode.data();
		shaderInfo.pName = "main";
		
		shaderInfo.codeType = vk::ShaderCodeTypeEXT::eSpirv;
		shaderInfo.stage = vk::ShaderStageFlagBits::eVertex;
		shaderInfo.nextStage = vk::ShaderStageFlagBits::eFragment;

		vk::DescriptorSetLayout layouts[2];
		{

			vk::DescriptorSetLayoutBinding binding = {};
			binding.binding = 0;
			binding.descriptorCount = 1;
			binding.descriptorType = vk::DescriptorType::eUniformBuffer;
			binding.stageFlags = vk::ShaderStageFlagBits::eVertex;

			vk::DescriptorSetLayoutCreateInfo layoutInfo = {};
			layoutInfo.bindingCount = 1;
			layoutInfo.pBindings = &binding;

			layouts[0] = m_pCore->getDevice().createDescriptorSetLayout(layoutInfo);
		}
		{
			vk::DescriptorSetLayoutBinding binding = {};
			binding.binding = 0;
			binding.descriptorCount = 1;
			binding.descriptorType = vk::DescriptorType::eUniformBuffer;
			binding.stageFlags = vk::ShaderStageFlagBits::eVertex;

			vk::DescriptorSetLayoutCreateInfo layoutInfo = {};
			layoutInfo.bindingCount = 1;
			layoutInfo.pBindings = &binding;

			layouts[1] = m_pCore->getDevice().createDescriptorSetLayout(layoutInfo);
		}

		shaderInfo.pSetLayouts = layouts;
		shaderInfo.setLayoutCount = 2;
		shaderInfo.pushConstantRangeCount = 0;
		shaderInfo.pPushConstantRanges = nullptr;

		//auto shader = m_pCore->getDevice().createShaderEXT(shaderInfo);
		VkShaderCreateInfoEXT vkInfo = static_cast<VkShaderCreateInfoEXT>(shaderInfo);
		VkShaderEXT shader;

		VkResult result = vkCreateShadersEXT(m_pCore->getDevice(), 1, &vkInfo, nullptr, &shader);
		/*
		
		auto fun = (PFN_vkCreateShadersEXT)vkGetDeviceProcAddr(m_pCore->getDevice(), "vkCreateShadersEXT");
		
		VkShaderEXT shader;

		VkShaderCreateInfoEXT vkInfo = static_cast<VkShaderCreateInfoEXT>(shaderInfo);
		VkResult result = fun(m_pCore->getDevice(), 1, &vkInfo, nullptr, &shader);
		if (result != VK_SUCCESS) {
			std::cout << "Failed to create shader" << std::endl;
		}
		*/
		m_shader = ResourceManager::get().insert(static_cast<vk::ShaderEXT>(shader));
	}

	ResourceID Shader::getShaderID() const {
		return m_shader;
	}
}
