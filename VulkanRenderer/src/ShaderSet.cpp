#include "pch.h"
#include "Resources/ShaderSet.hpp"

namespace sa {
	void ShaderSet::init(const std::vector<Shader>& shaders) {
		size_t descriptorSetLayoutSize = 0;
		for (const auto& shader : shaders) {
			if (descriptorSetLayoutSize < shader.getDescriptorSetLayouts().size()) {
				descriptorSetLayoutSize = shader.getDescriptorSetLayouts().size();
			}
		}

		m_descriptorSetLayouts.resize(descriptorSetLayoutSize);
		m_descriptorSets.resize(m_descriptorSetLayouts.size());
		for (uint32_t setIndex = 0; setIndex < m_descriptorSetLayouts.size(); setIndex++) {
			// merge all shaders bindings
			for (const auto& shader : shaders) {
				if (shader.getDescriptorSetLayouts().size() > setIndex) {
					const auto& set = shader.getDescriptorSetLayouts()[setIndex];

					m_descriptorSets[setIndex].bindings.insert(m_descriptorSets[setIndex].bindings.end(), set.bindings.begin(), set.bindings.end());
					m_descriptorSets[setIndex].sizes.insert(m_descriptorSets[setIndex].sizes.end(), set.sizes.begin(), set.sizes.end());
					m_descriptorSets[setIndex].writes.insert(m_descriptorSets[setIndex].writes.end(), set.writes.begin(), set.writes.end());
				}

			
			}

			// erase duplicate bindings
			for (int i = 0; i < m_descriptorSets[setIndex].bindings.size() - 1; i++) {
				for (int j = i + 1; j < m_descriptorSets[setIndex].bindings.size(); j++) {
					if (m_descriptorSets[setIndex].bindings[i].binding == m_descriptorSets[setIndex].bindings[j].binding) {
						m_descriptorSets[setIndex].bindings.erase(m_descriptorSets[setIndex].bindings.begin() + j);
						m_descriptorSets[setIndex].writes.erase(m_descriptorSets[setIndex].writes.begin() + j);
						m_descriptorSets[setIndex].sizes.erase(m_descriptorSets[setIndex].sizes.begin() + j);
						j--;
					}
				}
			}


			// to support variable descriptor counts
			vk::DescriptorSetLayoutBindingFlagsCreateInfo flagCreateInfo;
			
			std::vector<vk::DescriptorBindingFlags> flags(m_descriptorSets[setIndex].bindings.size(), (vk::DescriptorBindingFlags)0);

			for (size_t i = 0; i < flags.size(); i++) {
				if (m_descriptorSets[setIndex].bindings[i].descriptorCount == 0) {
					m_descriptorSets[setIndex].bindings[i].descriptorCount = MAX_VARIABLE_DESCRIPTOR_COUNT;
					flags[i] = vk::DescriptorBindingFlagBits::eVariableDescriptorCount | vk::DescriptorBindingFlagBits::ePartiallyBound;
					if (m_descriptorSets[setIndex].bindings[i].binding != m_descriptorSets[setIndex].bindings.size() - 1) {
						throw std::runtime_error("Variable count descriptors has to be on the last binding");
					}
				}
			}
			flagCreateInfo.setBindingFlags(flags);


			// Create descriptorSet layout create info
			vk::DescriptorSetLayoutCreateInfo layoutInfo;
			layoutInfo.setPNext(&flagCreateInfo);

			layoutInfo.setBindings(m_descriptorSets[setIndex].bindings);
			m_descriptorSetLayouts[setIndex] = m_device.createDescriptorSetLayout(layoutInfo);
		}

		std::set<vk::DescriptorType> descriptorTypes;
		std::vector<vk::DescriptorPoolSize> poolSizes;
		for (const auto& shader : shaders) {
			m_pushConstantRanges.insert(m_pushConstantRanges.end(), shader.getPushConstantRanges().begin(), shader.getPushConstantRanges().end());
			m_shaderInfos.push_back(shader.getInfo());

			for (const auto& set : shader.getDescriptorSetLayouts()) {
				for (const auto& binding : set.bindings) {
					if (descriptorTypes.find(binding.descriptorType) == descriptorTypes.end()) {
						vk::DescriptorPoolSize poolSize = {};
						poolSize.descriptorCount = UINT16_MAX;
						poolSize.type = binding.descriptorType;
						poolSizes.push_back(poolSize);
						descriptorTypes.insert(binding.descriptorType);
					}
				}
			}
		}
		

		m_descriptorPool = nullptr;
		if (poolSizes.size() > 0) {
			vk::DescriptorPoolCreateInfo poolInfo{
				.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
				.maxSets = UINT16_MAX,
			};
			poolInfo.setPoolSizes(poolSizes);
			m_descriptorPool = m_device.createDescriptorPool(poolInfo);
		}
	}

	ShaderSet::ShaderSet(vk::Device device, const Shader& vertexShader, const Shader& fragmentShader)
		: m_device(device)
		, m_isGraphicsSet(true)
		, m_vertexShader(vertexShader)
		, m_fragmentShader(fragmentShader)
	{
		if ((vertexShader.getStage() | fragmentShader.getStage()) != (vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment)) {
			throw std::runtime_error("Missing stages");
		}


		init({ vertexShader, fragmentShader });

		m_vertexAttributes = vertexShader.getVertexAttributes();
		m_vertexBindings = vertexShader.getVertexBindings();

		//m_descriptorSetLayouts.resize(std::max(vertexShader.getDescriptorSetLayouts().size(), fragmentShader.getDescriptorSetLayouts().size()));
		//m_descriptorSets.resize(m_descriptorSetLayouts.size());
		//for (uint32_t setIndex = 0; setIndex < m_descriptorSetLayouts.size(); setIndex++) {
		//	if (vertexShader.getDescriptorSetLayouts().size() > setIndex) {
		//		const auto& vset = vertexShader.getDescriptorSetLayouts()[setIndex];
		//		
		//		m_descriptorSets[setIndex].bindings.insert(m_descriptorSets[setIndex].bindings.end(), vset.bindings.begin(), vset.bindings.end());
		//		m_descriptorSets[setIndex].sizes.insert(m_descriptorSets[setIndex].sizes.end(), vset.sizes.begin(), vset.sizes.end());
		//		m_descriptorSets[setIndex].writes.insert(m_descriptorSets[setIndex].writes.end(), vset.writes.begin(), vset.writes.end());

		//	}
		//	if (fragmentShader.getDescriptorSetLayouts().size() > setIndex) {
		//		const auto& fset = fragmentShader.getDescriptorSetLayouts()[setIndex];

		//		m_descriptorSets[setIndex].bindings.insert(m_descriptorSets[setIndex].bindings.end(), fset.bindings.begin(), fset.bindings.end());
		//		m_descriptorSets[setIndex].sizes.insert(m_descriptorSets[setIndex].sizes.end(), fset.sizes.begin(), fset.sizes.end());
		//		m_descriptorSets[setIndex].writes.insert(m_descriptorSets[setIndex].writes.end(), fset.writes.begin(), fset.writes.end());

		//	}

		//	// erase duplicate bindings
		//	for (int i = 0; i < m_descriptorSets[setIndex].bindings.size() - 1; i++) {
		//		for (int j = i + 1; j < m_descriptorSets[setIndex].bindings.size(); j++) {
		//			if (m_descriptorSets[setIndex].bindings[i].binding == m_descriptorSets[setIndex].bindings[j].binding) {
		//				m_descriptorSets[setIndex].bindings.erase(m_descriptorSets[setIndex].bindings.begin() + j);
		//				m_descriptorSets[setIndex].writes.erase(m_descriptorSets[setIndex].writes.begin() + j);
		//				m_descriptorSets[setIndex].sizes.erase(m_descriptorSets[setIndex].sizes.begin() + j);
		//				j--;
		//			}
		//		}
		//	}

		//	// to support variable descriptor counts
		//	vk::DescriptorSetLayoutBindingFlagsCreateInfo flagCreateInfo;
		//	flagCreateInfo.bindingCount = m_descriptorSets[setIndex].bindings.size();
		//	std::vector<vk::DescriptorBindingFlags> flags(flagCreateInfo.bindingCount, (vk::DescriptorBindingFlags)0);
		//	for (size_t i = 0; i < flagCreateInfo.bindingCount; i++) {
		//		if (m_descriptorSets[setIndex].bindings[i].descriptorCount == 0) {
		//			flags[i] = vk::DescriptorBindingFlagBits::eVariableDescriptorCount;
		//		}	
		//	}
		//	flagCreateInfo.setBindingFlags(flags);


		//	vk::DescriptorSetLayoutCreateInfo layoutInfo;
		//	layoutInfo.setPNext(&flagCreateInfo);

		//	layoutInfo.setBindings(m_descriptorSets[setIndex].bindings);
		//	m_descriptorSetLayouts[setIndex] = m_device.createDescriptorSetLayout(layoutInfo);
		//}
	
		//m_pushConstantRanges = vertexShader.getPushConstantRanges();
		//m_pushConstantRanges.insert(m_pushConstantRanges.end(), fragmentShader.getPushConstantRanges().begin(), fragmentShader.getPushConstantRanges().end());
		//
		//m_shaderInfos.push_back(vertexShader.getInfo());
		//m_shaderInfos.push_back(fragmentShader.getInfo());

		//m_vertexAttributes = vertexShader.getVertexAttributes();
		//m_vertexBindings = vertexShader.getVertexBindings();


		//std::set<vk::DescriptorType> descriptorTypes;
		//std::vector<vk::DescriptorPoolSize> poolSizes;
		//for (const auto& set : vertexShader.getDescriptorSetLayouts()) {
		//	for (const auto& binding : set.bindings) {
		//		if (descriptorTypes.find(binding.descriptorType) == descriptorTypes.end()) {
		//			vk::DescriptorPoolSize poolSize = {};
		//			poolSize.descriptorCount = UINT16_MAX;
		//			poolSize.type = binding.descriptorType;
		//			poolSizes.push_back(poolSize);
		//			descriptorTypes.insert(binding.descriptorType);
		//		}
		//	}
		//}
		//for (const auto& set : fragmentShader.getDescriptorSetLayouts()) {
		//	for (const auto& binding : set.bindings) {
		//		if (descriptorTypes.find(binding.descriptorType) == descriptorTypes.end()) {
		//			vk::DescriptorPoolSize poolSize = {};
		//			poolSize.descriptorCount = UINT16_MAX;
		//			poolSize.type = binding.descriptorType;
		//			poolSizes.push_back(poolSize);
		//			descriptorTypes.insert(binding.descriptorType);
		//		}
		//	}
		//}

		//m_descriptorPool = nullptr;
		//if (poolSizes.size() > 0) {
		//	vk::DescriptorPoolCreateInfo poolInfo{
		//		.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
		//		.maxSets = UINT16_MAX,
		//	};
		//	poolInfo.setPoolSizes(poolSizes);
		//	m_descriptorPool = m_device.createDescriptorPool(poolInfo);
		//}
	}

	ShaderSet::ShaderSet(vk::Device device, const Shader& vertexShader, const Shader& geometryShader, const Shader& fragmentShader)
		: m_device(device)
		, m_isGraphicsSet(true)
		, m_vertexShader(vertexShader)
		, m_geometryShader(geometryShader)
		, m_fragmentShader(fragmentShader)
	{
		if ((vertexShader.getStage() | geometryShader.getStage() | fragmentShader.getStage()) != (vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eGeometry | vk::ShaderStageFlagBits::eFragment)) {
			throw std::runtime_error("Missing stages");
		}

		m_descriptorSetLayouts.resize(std::max(geometryShader.getDescriptorSetLayouts().size(), std::max(vertexShader.getDescriptorSetLayouts().size(), fragmentShader.getDescriptorSetLayouts().size())));
		m_descriptorSets.resize(m_descriptorSetLayouts.size());
		for (uint32_t setIndex = 0; setIndex < m_descriptorSetLayouts.size(); setIndex++) {
			std::vector<vk::DescriptorSetLayoutBinding> bindings;
			if (vertexShader.getDescriptorSetLayouts().size() > setIndex) {
				const auto& vset = vertexShader.getDescriptorSetLayouts()[setIndex];

				m_descriptorSets[setIndex].bindings.insert(m_descriptorSets[setIndex].bindings.end(), vset.bindings.begin(), vset.bindings.end());
				m_descriptorSets[setIndex].sizes.insert(m_descriptorSets[setIndex].sizes.end(), vset.sizes.begin(), vset.sizes.end());
				m_descriptorSets[setIndex].writes.insert(m_descriptorSets[setIndex].writes.end(), vset.writes.begin(), vset.writes.end());

				bindings.insert(bindings.end(), vset.bindings.begin(), vset.bindings.end());
			}
			if (geometryShader.getDescriptorSetLayouts().size() > setIndex) {
				const auto& gset = geometryShader.getDescriptorSetLayouts()[setIndex];
				
				m_descriptorSets[setIndex].bindings.insert(m_descriptorSets[setIndex].bindings.end(), gset.bindings.begin(), gset.bindings.end());
				m_descriptorSets[setIndex].sizes.insert(m_descriptorSets[setIndex].sizes.end(), gset.sizes.begin(), gset.sizes.end());
				m_descriptorSets[setIndex].writes.insert(m_descriptorSets[setIndex].writes.end(), gset.writes.begin(), gset.writes.end());
				
				bindings.insert(bindings.end(), gset.bindings.begin(), gset.bindings.end());
			}
			if (fragmentShader.getDescriptorSetLayouts().size() > setIndex) {
				const auto& fset = fragmentShader.getDescriptorSetLayouts()[setIndex];

				m_descriptorSets[setIndex].bindings.insert(m_descriptorSets[setIndex].bindings.end(), fset.bindings.begin(), fset.bindings.end());
				m_descriptorSets[setIndex].sizes.insert(m_descriptorSets[setIndex].sizes.end(), fset.sizes.begin(), fset.sizes.end());
				m_descriptorSets[setIndex].writes.insert(m_descriptorSets[setIndex].writes.end(), fset.writes.begin(), fset.writes.end());

				bindings.insert(bindings.end(), fset.bindings.begin(), fset.bindings.end());
			}

			vk::DescriptorSetLayoutCreateInfo layoutInfo;
			layoutInfo.setBindings(bindings);
			m_descriptorSetLayouts[setIndex] = m_device.createDescriptorSetLayout(layoutInfo);

		}

		m_pushConstantRanges = vertexShader.getPushConstantRanges();
		m_pushConstantRanges.insert(m_pushConstantRanges.end(), fragmentShader.getPushConstantRanges().begin(), fragmentShader.getPushConstantRanges().end());
		m_pushConstantRanges.insert(m_pushConstantRanges.end(), geometryShader.getPushConstantRanges().begin(), geometryShader.getPushConstantRanges().end());

		m_shaderInfos.push_back(vertexShader.getInfo());
		m_shaderInfos.push_back(fragmentShader.getInfo());
		m_shaderInfos.push_back(geometryShader.getInfo());

		m_vertexAttributes = vertexShader.getVertexAttributes();
		m_vertexBindings = vertexShader.getVertexBindings();

		std::set<vk::DescriptorType> descriptorTypes;
		std::vector<vk::DescriptorPoolSize> poolSizes;
		for (const auto& set : vertexShader.getDescriptorSetLayouts()) {
			for (const auto& binding : set.bindings) {
				if (descriptorTypes.find(binding.descriptorType) == descriptorTypes.end()) {
					vk::DescriptorPoolSize poolSize = {};
					poolSize.descriptorCount = UINT16_MAX;
					poolSize.type = binding.descriptorType;
					poolSizes.push_back(poolSize);
					descriptorTypes.insert(binding.descriptorType);
				}
			}
		}
		for (const auto& set : fragmentShader.getDescriptorSetLayouts()) {
			for (const auto& binding : set.bindings) {
				if (descriptorTypes.find(binding.descriptorType) == descriptorTypes.end()) {
					vk::DescriptorPoolSize poolSize = {};
					poolSize.descriptorCount = UINT16_MAX;
					poolSize.type = binding.descriptorType;
					poolSizes.push_back(poolSize);
					descriptorTypes.insert(binding.descriptorType);
				}
			}
		}
		for (const auto& set : geometryShader.getDescriptorSetLayouts()) {
			for (const auto& binding : set.bindings) {
				if (descriptorTypes.find(binding.descriptorType) == descriptorTypes.end()) {
					vk::DescriptorPoolSize poolSize = {};
					poolSize.descriptorCount = UINT16_MAX;
					poolSize.type = binding.descriptorType;
					poolSizes.push_back(poolSize);
					descriptorTypes.insert(binding.descriptorType);
				}
			}
		}

		m_descriptorPool = nullptr;
		if (poolSizes.size() > 0) {
			vk::DescriptorPoolCreateInfo poolInfo{
				.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
				.maxSets = UINT16_MAX,
			};
			poolInfo.setPoolSizes(poolSizes);
			m_descriptorPool = m_device.createDescriptorPool(poolInfo);
		}

	}

	ShaderSet::ShaderSet(vk::Device device, const Shader& computeShader)
		: m_device(device)
		, m_isGraphicsSet(false)
		, m_computeShader(computeShader)
	{
		if (computeShader.getStage() != vk::ShaderStageFlagBits::eCompute) {
			throw std::runtime_error("Missing compute stage");
		}

		m_descriptorSetLayouts.resize(computeShader.getDescriptorSetLayouts().size());
		m_descriptorSets.resize(m_descriptorSetLayouts.size());
		for (uint32_t setIndex = 0; setIndex < m_descriptorSetLayouts.size(); setIndex++) {
			const auto& set = computeShader.getDescriptorSetLayouts()[setIndex];

			m_descriptorSets[setIndex].bindings = set.bindings;
			m_descriptorSets[setIndex].sizes = set.sizes;
			m_descriptorSets[setIndex].writes = set.writes;

		
			vk::DescriptorSetLayoutCreateInfo layoutInfo;
			layoutInfo.setBindings(set.bindings);
			m_descriptorSetLayouts[setIndex] = m_device.createDescriptorSetLayout(layoutInfo);
		}

		m_pushConstantRanges = computeShader.getPushConstantRanges();
		
		m_shaderInfos.push_back(computeShader.getInfo());
		
		std::set<vk::DescriptorType> descriptorTypes;
		std::vector<vk::DescriptorPoolSize> poolSizes;
		for (const auto& set : computeShader.getDescriptorSetLayouts()) {
			for (const auto& binding : set.bindings) {
				if (descriptorTypes.find(binding.descriptorType) == descriptorTypes.end()) {
					vk::DescriptorPoolSize poolSize = {};
					poolSize.descriptorCount = UINT16_MAX;
					poolSize.type = binding.descriptorType;
					poolSizes.push_back(poolSize);
					descriptorTypes.insert(binding.descriptorType);
				}
			}
		}

		m_descriptorPool = nullptr;
		if (poolSizes.size() > 0) {
			vk::DescriptorPoolCreateInfo poolInfo{
				.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
				.maxSets = UINT16_MAX,
			};
			poolInfo.setPoolSizes(poolSizes);
			m_descriptorPool = m_device.createDescriptorPool(poolInfo);
		}

	}

	void ShaderSet::destroy() {

		if (m_vertexShader.has_value())
			m_vertexShader->destroy();
		
		if (m_geometryShader.has_value()) 
			m_geometryShader->destroy();
		
		if (m_fragmentShader.has_value()) 
			m_fragmentShader->destroy();
		
		if (m_computeShader.has_value())
			m_computeShader->destroy();


		for (auto& layout : m_descriptorSetLayouts) {
			m_device.destroyDescriptorSetLayout(layout);
		}
		if (m_descriptorPool) {
			m_device.destroyDescriptorPool(m_descriptorPool);
		}
	}

	const std::vector<vk::DescriptorSetLayout>& ShaderSet::getDescriptorSetLayouts() const {
		return m_descriptorSetLayouts;
	}

	const std::vector<vk::PushConstantRange>& ShaderSet::getPushConstantRanges() const {
		return m_pushConstantRanges;
	}

	const std::vector<vk::PipelineShaderStageCreateInfo>& ShaderSet::getShaderInfos() const {
		return m_shaderInfos;
	}

	const std::vector<vk::VertexInputAttributeDescription>& ShaderSet::getVertexAttributes() const {
		return m_vertexAttributes;
	}

	const std::vector<vk::VertexInputBindingDescription>& ShaderSet::getVertexBindings() const {
		return m_vertexBindings;
	}

	bool ShaderSet::isGraphicsSet() const {
		return m_isGraphicsSet;
	}

	DescriptorSet ShaderSet::allocateDescriptorSet(uint32_t setIndex, uint32_t count) {
		if (m_descriptorSetLayouts.size() <= setIndex) {
			throw std::runtime_error("Set index out of bounds!");
		}
		DescriptorSet descriptorSet;
		descriptorSet.create(m_device, m_descriptorPool, count, m_descriptorSets[setIndex], m_descriptorSetLayouts[setIndex], setIndex);
		return descriptorSet;
	}

}
