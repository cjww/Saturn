#include "pch.h"
#include "Shader.hpp"
#include "internal/VulkanCore.hpp"
#include "internal/DescriptorSet.hpp"

#include "vulkan\vulkan.hpp"

#include "Renderer.hpp"

#include <spirv_cross/spirv_cross.hpp>

#include <filesystem>

namespace sa {
	void Shader::createModule() {
		vk::ShaderModuleCreateInfo createInfo = {};
		createInfo.pCode = m_stageInfo.pCode;
		createInfo.codeSize = m_stageInfo.codeLength * sizeof(uint32_t);
		const vk::ShaderModule shader = m_pCore->getDevice().createShaderModule(createInfo);
		m_shaderModule = ResourceManager::get().insert(shader);
	}

	void Shader::createObject(ShaderStageFlags nextStages, const PipelineLayout& layout) {
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

		shaderInfo.codeSize = m_stageInfo.codeLength * sizeof(uint32_t);
		shaderInfo.pCode = m_stageInfo.pCode;
		shaderInfo.pName = m_stageInfo.pName;

		shaderInfo.codeType = vk::ShaderCodeTypeEXT::eSpirv;
		shaderInfo.stage = static_cast<vk::ShaderStageFlagBits>(m_stageInfo.stage);
		shaderInfo.nextStage = static_cast<vk::ShaderStageFlags>(nextStages);

		/*
		vk::DescriptorSetLayout descriptorSetLayout;
		{
			vk::DescriptorSetLayoutBinding binding = {};
			binding.binding = 0;
			binding.descriptorCount = 1;
			binding.descriptorType = vk::DescriptorType::eUniformBuffer;
			binding.stageFlags = vk::ShaderStageFlagBits::eVertex;

			vk::DescriptorSetLayoutCreateInfo layoutInfo = {};
			layoutInfo.bindingCount = 1;
			layoutInfo.pBindings = &binding;

			descriptorSetLayout = m_pCore->getDevice().createDescriptorSetLayout(layoutInfo);
		}

		std::vector<vk::PushConstantRange> ranges;
		for (const auto& range : layout.getPushConstantRanges()) {
			vk::PushConstantRange r = {};
			r.stageFlags = static_cast<vk::ShaderStageFlags>(range.stageFlags);
			r.offset = range.offset;
			r.size = range.size;

			ranges.push_back(r);
		}
		*/

		const auto& descriptorSetLayouts = layout.getDescriptorSetLayouts();
		std::vector<vk::DescriptorSetLayout> vkDescriptorSetLayouts;
		vkDescriptorSetLayouts.reserve(descriptorSetLayouts.size());
		for(const auto& [index, set] : descriptorSetLayouts) {
			vk::DescriptorSetLayout* pLayout = ResourceManager::get().get<vk::DescriptorSetLayout>(set);
			if (!pLayout)
				throw std::runtime_error("Invalid DescriptorSetLayout ID");
			vkDescriptorSetLayouts.push_back(*pLayout);
		}

		const auto& pushConstantRanges = layout.getPushConstantRanges();
		std::vector<vk::PushConstantRange> vkPushConstantRanges;
		vkPushConstantRanges.reserve(pushConstantRanges.size());
		for(const auto& range : pushConstantRanges) {
			vk::PushConstantRange vkRange = {};
			vkRange.stageFlags = static_cast<vk::ShaderStageFlags>(range.stageFlags);
			vkRange.size = range.size;
			vkRange.offset = range.offset;
			vkPushConstantRanges.push_back(vkRange);
		}

		shaderInfo.pSetLayouts = vkDescriptorSetLayouts.data();
		shaderInfo.setLayoutCount = vkDescriptorSetLayouts.size();
		shaderInfo.pPushConstantRanges = vkPushConstantRanges.data();
		shaderInfo.pushConstantRangeCount = vkPushConstantRanges.size();
		
		const VkShaderCreateInfoEXT vkInfo = static_cast<VkShaderCreateInfoEXT>(shaderInfo);
		VkShaderEXT shader;
		VkResult result = vkCreateShadersEXT(m_pCore->getDevice(), 1, &vkInfo, nullptr, &shader);
		checkError(static_cast<vk::Result>(result), "Failed to create shader");

		m_shaderObject = ResourceManager::get().insert(shader);
	}

	void Shader::initShaderStageInfo(const std::vector<uint32_t>& shaderCode, ShaderStageFlagBits stage,
		const char* entryPointName)
	{
		m_stageInfo.codeLength = shaderCode.size();
		uint32_t* code = new uint32_t[m_stageInfo.codeLength];
		memcpy(code, shaderCode.data(), m_stageInfo.codeLength * sizeof(uint32_t));
		m_stageInfo.pCode = code;
		m_stageInfo.stage = stage;
		m_stageInfo.pName = entryPointName;
	}

	Shader::Shader()
		: m_pCore(Renderer::get().getCore())
		, m_stageInfo({ 
			.pName = "main",
			.pCode = nullptr,
			.codeLength = 0,
			.stage = static_cast<ShaderStageFlagBits>(0)
		})
	{

	}

	Shader::~Shader() {
		if (m_stageInfo.pCode != nullptr) {
			delete m_stageInfo.pCode;
			m_stageInfo.pCode = nullptr;
		}
	}

	Shader::Shader(const Shader& other) {
		m_pCore = other.m_pCore;
		m_stageInfo = other.m_stageInfo;
		uint32_t* code = new uint32_t[other.m_stageInfo.codeLength];
		memcpy(code, other.m_stageInfo.pCode, m_stageInfo.codeLength * sizeof(uint32_t));
		m_stageInfo.pCode = code;
		m_shaderModule = other.m_shaderModule;
		m_shaderObject = other.m_shaderObject;
	}

	Shader& Shader::operator=(const Shader& other) {
		this->~Shader();
		m_pCore = other.m_pCore;
		m_stageInfo = other.m_stageInfo;
		uint32_t* code = new uint32_t[other.m_stageInfo.codeLength];
		memcpy(code, other.m_stageInfo.pCode, m_stageInfo.codeLength * sizeof(uint32_t));
		m_stageInfo.pCode = code;
		m_shaderModule = other.m_shaderModule;
		m_shaderObject = other.m_shaderObject;
		return *this;
	}

	void Shader::create(const ShaderStageInfo& stageInfo) {
		m_stageInfo = stageInfo;
		createModule();
	}

	void Shader::create(const std::vector<uint32_t>& shaderCode, const char* entryPointName) {
		spirv_cross::Compiler compiler(shaderCode);
		const auto entryPoints = compiler.get_entry_points_and_stages();

		ShaderStageFlagBits stage = (ShaderStageFlagBits)0;
		for(const auto& ep : entryPoints) {
			if(ep.name == entryPointName) {
				stage = ShaderStageFlagBits(1U << (uint32_t)ep.execution_model);
				break;
			}
		}
		if(stage == 0){
			throw std::runtime_error(std::string("entryPoint not found: ") + entryPointName);
		}
		initShaderStageInfo(shaderCode, stage, entryPointName);
		createModule();
	}

	void Shader::create(const std::vector<uint32_t>& shaderCode, ShaderStageFlagBits stage, const char* entryPointName) {
		initShaderStageInfo(shaderCode, stage, entryPointName);
		createModule();
	}

	void Shader::create(const std::vector<uint32_t>& shaderCode, ShaderStageFlagBits stage, ShaderStageFlags nextStages, const PipelineLayout& layout) {
		initShaderStageInfo(shaderCode, stage, "main");
		createObject(nextStages, layout);
	}

	bool Shader::isValid() const {
		return m_shaderModule != NULL_RESOURCE || m_shaderObject != NULL_RESOURCE;
	}

	void Shader::destroy() {
		if(m_shaderModule != NULL_RESOURCE) {
			ResourceManager::get().remove<vk::ShaderModule>(m_shaderModule);
		}
		m_shaderModule = NULL_RESOURCE;
		if (m_shaderObject != NULL_RESOURCE) {
			ResourceManager::get().remove<VkShaderEXT>(m_shaderModule);
		}
		m_shaderObject = NULL_RESOURCE;

		if(m_stageInfo.pCode != nullptr) {
			delete m_stageInfo.pCode;
			m_stageInfo.pCode = nullptr;
		}
	}

	ResourceID Shader::getShaderModuleID() const {
		return m_shaderModule;
	}

	ResourceID Shader::getShaderObjectID() const {
		return m_shaderObject;
	}

	const ShaderStageInfo& Shader::getShaderStageInfo() const {
		return m_stageInfo;
	}

	ShaderStageFlagBits Shader::getStage() const {
		return m_stageInfo.stage;
	}
}
