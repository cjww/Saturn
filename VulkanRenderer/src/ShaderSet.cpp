#include "pch.h"
#include "internal/ShaderSet.hpp"
#include "internal/VulkanCore.hpp"
#include "internal/DescriptorSet.hpp"

#include "Renderer.hpp"

#define NOMINMAX
#include <spirv_cross/spirv_cross.hpp>

#include <shaderc\shaderc.hpp>

namespace sa {

	shaderc_shader_kind ToShadercKind(ShaderStageFlagBits shaderStage) {
		static const std::unordered_map<ShaderStageFlagBits, shaderc_shader_kind> map = {
			{ShaderStageFlagBits::VERTEX, shaderc_glsl_default_vertex_shader },
			{ShaderStageFlagBits::FRAGMENT, shaderc_glsl_default_fragment_shader},
			{ShaderStageFlagBits::COMPUTE, shaderc_glsl_default_compute_shader},
			{ShaderStageFlagBits::GEOMETRY, shaderc_glsl_default_geometry_shader},
			{ShaderStageFlagBits::TESSELATION_CONTROL, shaderc_glsl_default_tess_control_shader},
			{ShaderStageFlagBits::TESSELATION_EVALUATION, shaderc_glsl_default_tess_evaluation_shader},
		};
		return map.at(shaderStage);
	}

	void createAttribute(const std::string& name, const spirv_cross::SPIRType& type, ShaderAttribute& attrib) {
		attrib.name = name;
		std::copy(type.array.begin(), type.array.end(), std::back_inserter(attrib.arraySize));
		attrib.vecSize = type.vecsize;
		attrib.columns = type.columns;
		attrib.type = (ShaderAttributeType)type.basetype;
	}

	void addStructMember(
		const spirv_cross::Compiler* pCompiler,
		const spirv_cross::SPIRType& parentType,
		int index,
		DescriptorType descriptorType,
		uint32_t set,
		uint32_t binding,
		uint32_t offset,
		std::unordered_map<std::string, ShaderAttribute>& outAttributes,
		const std::string& attributePath)
	{
		const auto& type = pCompiler->get_type(parentType.member_types[index]);

		std::string name = pCompiler->get_member_name(parentType.self, index);

		if (outAttributes.count(attributePath + name)) // already added
			return;

		ShaderAttribute& attrib = outAttributes[attributePath + name];
		createAttribute(name, type, attrib);

		attrib.descriptorType = descriptorType;
		attrib.set = set;
		attrib.binding = binding;

		attrib.offset = pCompiler->type_struct_member_offset(parentType, index) + offset;
		attrib.size = pCompiler->get_declared_struct_member_size(parentType, index);


		int i = 0;
		for (spirv_cross::TypeID memberTypeID : type.member_types) {
			addStructMember(pCompiler, type, i, descriptorType, set, binding, attrib.offset, outAttributes, (attrib.name.empty() ? attributePath : attributePath + attrib.name + "."));
			i++;
		}
	}

	void addBlockMember(
		const spirv_cross::Compiler* pCompiler, 
		const spirv_cross::SPIRType& baseType,
		const spirv_cross::SPIRType& parentType,
		const spirv_cross::BufferRange& range,
		DescriptorType descriptorType,
		uint32_t set,
		uint32_t binding,
		std::unordered_map<std::string, ShaderAttribute>& outAttributes,
		const std::string& attributePath)
	{
		const auto& type = pCompiler->get_type(parentType.member_types[range.index]);

		std::string name = pCompiler->get_member_name(parentType.self, range.index);

		if (outAttributes.count(attributePath + name)) // already added
			return;

		ShaderAttribute& attrib = outAttributes[attributePath + name];
		createAttribute(name, type, attrib);

		attrib.descriptorType = descriptorType;
		attrib.set = set;
		attrib.binding = binding;
		attrib.offset = range.offset;
		attrib.size = range.range;

		int i = 0;
		for (spirv_cross::TypeID memberTypeID : type.member_types) {
			addStructMember(pCompiler, type, i, descriptorType, set, binding, attrib.offset, outAttributes, (attrib.name.empty() ? attributePath : attributePath + attrib.name + "."));
			i++;
		}
		
	}

	void addResourceAttribute(
		const spirv_cross::Compiler* pCompiler,
		const spirv_cross::Resource& resource,
		DescriptorType descriptorType,
		std::unordered_map<std::string, ShaderAttribute>& outAttributes) 
	{
		const spirv_cross::SPIRType& type = pCompiler->get_type(resource.type_id);
		const spirv_cross::SPIRType& baseType = pCompiler->get_type(resource.base_type_id);

		std::string name = pCompiler->get_name(resource.id);
		ShaderAttribute& attrib = outAttributes[name];
		createAttribute(name, type, attrib);
		attrib.descriptorType = descriptorType;
		attrib.set = pCompiler->get_decoration(resource.id, spv::Decoration::DecorationDescriptorSet);
		attrib.binding = pCompiler->get_decoration(resource.id, spv::Decoration::DecorationBinding);
		attrib.offset = pCompiler->get_decoration(resource.id, spv::Decoration::DecorationOffset);

		bool isBlock = pCompiler->get_decoration_bitset(type.self).get(spv::Decoration::DecorationBlock) ||
			pCompiler->get_decoration_bitset(type.self).get(spv::Decoration::DecorationBufferBlock);
		
		std::string attributePath = attrib.name;

		if (isBlock) {
			attrib.size = pCompiler->get_declared_struct_size(type);
			const auto& bufferRanges = pCompiler->get_active_buffer_ranges(resource.id);
			for (const auto& range : bufferRanges) {
				addBlockMember(pCompiler, baseType, type, range, descriptorType, attrib.set, attrib.binding, outAttributes, attributePath.empty() ? "" : attributePath + ".");
			}
		}
		if(!attributePath.empty())
			outAttributes[attributePath] = attrib;
	}

	void addResources(
		const spirv_cross::Compiler* pCompiler,
		const spirv_cross::SmallVector<spirv_cross::Resource>& resources,
		DescriptorType type,
		vk::Sampler* immutableSamplers,
		ShaderStageFlagBits stage,
		std::unordered_map<uint32_t, DescriptorSetLayoutInfo>& descriptorSets,
		std::unordered_map<std::string, ShaderAttribute>& outAttributes)
	{
		for (auto& resource : resources) {
			const auto& t = pCompiler->get_type(resource.type_id);
			if ((type == DescriptorType::SAMPLED_IMAGE|| type == DescriptorType::COMBINED_IMAGE_SAMPLER|| type == DescriptorType::STORAGE_IMAGE) && t.image.dim == spv::Dim::DimBuffer) {
				continue; // this is not a sampled image
			}
			if ((type == DescriptorType::UNIFORM_TEXEL_BUFFER|| type == DescriptorType::STORAGE_TEXEL_BUFFER) && t.image.dim != spv::Dim::DimBuffer) {
				continue; // this is not a texel buffer
			}

			DescriptorSetLayoutBinding layoutBinding = {};
			uint32_t set = pCompiler->get_decoration(resource.id, spv::Decoration::DecorationDescriptorSet);
			layoutBinding.binding = pCompiler->get_decoration(resource.id, spv::Decoration::DecorationBinding);
			size_t size = 0;

			if (t.basetype == spirv_cross::SPIRType::BaseType::Struct) {
				size = pCompiler->get_declared_struct_size(t);
			}
			layoutBinding.stageFlags = stage;

			if (t.array.size() > 0) {
				layoutBinding.descriptorCount = t.array[0];
				if (layoutBinding.descriptorCount == 1) {
					layoutBinding.descriptorCount = 0;
				}
			}
			else {
				layoutBinding.descriptorCount = 1;
			}

			layoutBinding.type = type;
			layoutBinding.pImmutableSamplers = immutableSamplers;

			descriptorSets[set].bindings.push_back(layoutBinding);
			descriptorSets[set].sizes.push_back(size);

			addResourceAttribute(pCompiler, resource, type, outAttributes);
		}
	}

	void getVertexInput(
		const spirv_cross::Compiler* pCompiler,
		const spirv_cross::SmallVector<spirv_cross::Resource>& resources,
		std::vector<VertexInputAttributeDescription>& vertexAttributes,
		std::vector<VertexInputBindingDescription>& vertexBindings) {

		uint32_t size = 0;
		std::unordered_map<uint32_t, uint32_t> sizes;
		std::unordered_map<uint32_t, VertexInputAttributeDescription> attribs;

		for (auto& input : resources) {
			auto type = pCompiler->get_type(input.type_id);
			uint32_t location = pCompiler->get_decoration(input.id, spv::Decoration::DecorationLocation);
			uint32_t binding = pCompiler->get_decoration(input.id, spv::Decoration::DecorationBinding);
			Format format = Format::UNDEFINED;
			if (type.basetype == spirv_cross::SPIRType::BaseType::UInt) {
				format = Format::R32_UINT;
				sizes[location] = sizeof(unsigned int);
			}
			else if (type.basetype == spirv_cross::SPIRType::BaseType::Int || type.basetype == spirv_cross::SPIRType::BaseType::Boolean) {
				format = Format::R32_SINT;
				sizes[location] = sizeof(int);
			}
			else {
				int i = type.vecsize;
				switch (i) {
				case 1:
					format = Format::R32_SFLOAT;
					break;
				case 2:
					format = Format::R32G32_SFLOAT;
					break;
				case 3:
					format = Format::R32G32B32_SFLOAT;
					break;
				case 4:
					format = Format::R32G32B32A32_SFLOAT;
					break;
				}
				sizes[location] = sizeof(float) * i;

			}

			if (format == Format::UNDEFINED) {
				throw std::runtime_error("Undefined vertex input format");
			}

			VertexInputAttributeDescription attrib = {
				.location = location,
				.binding = binding,
				.format = format,
			};
			attribs[location] = attrib;

			size += sizes[location];
		}

		for (auto& input : resources) {
			uint32_t location = pCompiler->get_decoration(input.id, spv::Decoration::DecorationLocation);
			uint32_t offset = 0;
			for (uint32_t i = 0; i < location; i++) {
				offset += sizes[i];
			}
			attribs[location].offset = offset;
		}

		for (auto& it : attribs) {
			vertexAttributes.push_back(it.second);
		}
		if (vertexAttributes.size() > 0) {
			VertexInputBindingDescription b = {
				.binding = 0,
				.stride = size,
			};

			vertexBindings = { b };
		}
	}
	
	std::vector<uint32_t> CompileGLSLFromFile(const char* glslPath, ShaderStageFlagBits shaderStage, const char* entryPointName, const char* tag) {
		std::ifstream file(glslPath, std::ios::in);
		if (!file.is_open()) {
			throw std::runtime_error("Failed to open file " + std::string(glslPath));
		}

		file.ignore(std::numeric_limits<std::streamsize>::max());
		std::streamsize length = file.gcount();
		file.clear();   //  Since ignore will have set eof.
		file.seekg(0, std::ios_base::beg);

		std::string buffer;
		buffer.resize(length);
		file.read(buffer.data(), buffer.size());
		
		file.close();
		
		return CompileGLSLFromMemory(buffer.c_str(), shaderStage, entryPointName, tag);
	}

	std::vector<uint32_t> CompileGLSLFromMemory(const char* glslCode, ShaderStageFlagBits shaderStage, const char* entryPointName, const char* tag) {
		shaderc::CompileOptions options;
		options.SetAutoBindUniforms(true);
		options.SetAutoMapLocations(true);
		options.SetTargetEnvironment(shaderc_target_env_vulkan, SA_VK_API_VERSION);

		shaderc::Compiler compiler;
		shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(glslCode, ToShadercKind(shaderStage), tag, entryPointName, options);

		if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
			SA_DEBUG_LOG_ERROR(result.GetErrorMessage());
			return {};
		}

		std::vector<uint32_t> output;
		std::copy(result.begin(), result.end(), std::back_inserter(output));
		return std::move(output);
	}

	std::vector<uint32_t> ReadSPVFile(const char* spvPath) {
		std::ifstream file(spvPath, std::ios::binary);
		if (!file.is_open()) {
			throw std::runtime_error("Failed to open file " + std::string(spvPath));
		}
		file.ignore(std::numeric_limits<std::streamsize>::max());
		std::streamsize length = file.gcount();
		file.clear();
		file.seekg(0, std::ios_base::beg);

		std::vector<uint32_t> buffer(length / sizeof(uint32_t));
		file.read((char*)buffer.data(), length);
		
		file.close();

		return std::move(buffer);
	}

	void ShaderSet::createShaderModule(const ShaderStageInfo& stageInfo) {
		ShaderModuleInfo moduleInfo = {};
		moduleInfo.entryPointName = stageInfo.pName;
		moduleInfo.stage = stageInfo.stage;

		vk::ShaderModuleCreateInfo shaderInfo = {};
		shaderInfo.pCode = stageInfo.pCode;
		shaderInfo.codeSize = stageInfo.codeLength * sizeof(uint32_t);

		moduleInfo.moduleID = ResourceManager::get().insert<vk::ShaderModule>(m_pCore->getDevice().createShaderModule(shaderInfo));

		m_shaderModules.push_back(moduleInfo);
	}

	void ShaderSet::initializeStage(spirv_cross::Compiler* pCompiler, ShaderStageFlagBits stage) {
		if ((stage & sa::ShaderStageFlagBits::COMPUTE) != 0)
			m_isGraphicsSet = false;
		if ((stage & (sa::ShaderStageFlagBits::TESSELATION_CONTROL | sa::ShaderStageFlagBits::TESSELATION_EVALUATION)) != 0)
			m_hasTessellationStage = true;

		for (auto ext : pCompiler->get_declared_extensions()) {
			SA_DEBUG_LOG_INFO("Shader uses vulkan extension: ", ext);
		}

		auto resources = pCompiler->get_shader_resources();
		addResources(pCompiler, resources.uniform_buffers, DescriptorType::UNIFORM_BUFFER, nullptr, stage, m_descriptorSetLayoutInfos, m_attributes);
		addResources(pCompiler, resources.separate_samplers, DescriptorType::SAMPLER, nullptr, stage, m_descriptorSetLayoutInfos, m_attributes); // sampler / samplerShadow
		addResources(pCompiler, resources.separate_images, DescriptorType::SAMPLED_IMAGE, nullptr, stage, m_descriptorSetLayoutInfos, m_attributes); // texture2D
		addResources(pCompiler, resources.separate_images, DescriptorType::UNIFORM_TEXEL_BUFFER, nullptr, stage, m_descriptorSetLayoutInfos, m_attributes); // textureBuffer
		addResources(pCompiler, resources.sampled_images, DescriptorType::COMBINED_IMAGE_SAMPLER, nullptr, stage, m_descriptorSetLayoutInfos, m_attributes); // sampler2D
		addResources(pCompiler, resources.sampled_images, DescriptorType::UNIFORM_TEXEL_BUFFER, nullptr, stage, m_descriptorSetLayoutInfos, m_attributes); // samplerBuffer
		addResources(pCompiler, resources.storage_images, DescriptorType::STORAGE_IMAGE, nullptr, stage, m_descriptorSetLayoutInfos, m_attributes); // image2D
		addResources(pCompiler, resources.storage_images, DescriptorType::STORAGE_TEXEL_BUFFER, nullptr, stage, m_descriptorSetLayoutInfos, m_attributes); // 
		addResources(pCompiler, resources.storage_buffers, DescriptorType::STORAGE_BUFFER, nullptr, stage, m_descriptorSetLayoutInfos, m_attributes); // buffer SSBO 
		addResources(pCompiler, resources.subpass_inputs, DescriptorType::INPUT_ATTACHMENT, nullptr, stage, m_descriptorSetLayoutInfos, m_attributes); // subpassInput

		for (auto& p : resources.push_constant_buffers) {
			size_t size = pCompiler->get_declared_struct_size(pCompiler->get_type(p.type_id));
			uint32_t offset = pCompiler->type_struct_member_offset(pCompiler->get_type(p.base_type_id), 0);

			PushConstantRange range = {
				.stageFlags = stage,
				.offset = offset,
				.size = static_cast<uint32_t>(size),
			};
			m_pushConstantRanges.push_back(range);
			addResourceAttribute(pCompiler, p, DescriptorType::PUSH_CONSTANT, m_attributes);
		}

		if (stage & ShaderStageFlagBits::VERTEX) {
			m_vertexAttributes.clear();
			m_vertexBindings.clear();
			getVertexInput(pCompiler, resources.stage_inputs, m_vertexAttributes, m_vertexBindings);
		}
	}

	void ShaderSet::createDescriptorPoolAndLayouts() {
		std::set<vk::DescriptorType> descriptorTypes;
		std::vector<vk::DescriptorPoolSize> poolSizes;
		for (auto& [set, info] : m_descriptorSetLayoutInfos) {

			// to support variable descriptor counts
			vk::DescriptorSetLayoutBindingFlagsCreateInfo flagCreateInfo;

			std::vector<vk::DescriptorBindingFlags> flags(info.bindings.size(), (vk::DescriptorBindingFlags)0);

			for (size_t i = 0; i < flags.size(); i++) {
				if (info.bindings[i].descriptorCount == 0) {
					info.bindings[i].descriptorCount = MAX_VARIABLE_DESCRIPTOR_COUNT;
					flags[i] = vk::DescriptorBindingFlagBits::eVariableDescriptorCount | vk::DescriptorBindingFlagBits::ePartiallyBound;
					if (info.bindings[i].binding != info.bindings.size() - 1) {
						throw std::runtime_error("Variable count descriptors has to be on the last binding");
					}
				}
			}
			flagCreateInfo.setBindingFlags(flags);


			// Create descriptorSet layout create info
			vk::DescriptorSetLayoutCreateInfo layoutInfo;
			layoutInfo.setPNext(&flagCreateInfo);

			std::vector<vk::DescriptorSetLayoutBinding> layoutBindings(info.bindings.size());
			for (int i = 0; i < layoutBindings.size(); i++) {
				layoutBindings[i].binding = info.bindings[i].binding;
				layoutBindings[i].descriptorCount = info.bindings[i].descriptorCount;
				layoutBindings[i].descriptorType = (vk::DescriptorType)info.bindings[i].type;
				layoutBindings[i].pImmutableSamplers = info.bindings[i].pImmutableSamplers;
				layoutBindings[i].stageFlags = (vk::ShaderStageFlags)info.bindings[i].stageFlags;
			}

			layoutInfo.setBindings(layoutBindings);
			m_descriptorSetLayouts[set] = ResourceManager::get().insert<vk::DescriptorSetLayout>(m_pCore->getDevice().createDescriptorSetLayout(layoutInfo));

			for (const auto& binding : info.bindings) {
				if (!descriptorTypes.count((vk::DescriptorType)binding.type)) {
					vk::DescriptorPoolSize poolSize = {};
					poolSize.descriptorCount = UINT16_MAX;
					poolSize.type = (vk::DescriptorType)binding.type;
					poolSizes.push_back(poolSize);
					descriptorTypes.insert((vk::DescriptorType)binding.type);
				}
			}
		}

		m_descriptorPool = NULL_RESOURCE;
		if (poolSizes.size() > 0) {
			vk::DescriptorPoolCreateInfo poolInfo{
				.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
				.maxSets = UINT16_MAX,
			};
			poolInfo.setPoolSizes(poolSizes);
			m_descriptorPool = ResourceManager::get().insert<vk::DescriptorPool>(m_pCore->getDevice().createDescriptorPool(poolInfo));
		}
	}

	ShaderSet::~ShaderSet() {
		SA_DEBUG_LOG_INFO("Destroyed ShaderSet - ", (m_isGraphicsSet) ? "Graphics set " : "Compute set ", "Attribute Count: ", m_attributes.size());
	}

	ShaderSet::ShaderSet() 
		: m_isGraphicsSet(true)
		, m_hasTessellationStage(false)
		, m_pCore(Renderer::get().getCore())
	{
	}


	ShaderSet::ShaderSet(const ShaderStageInfo* pStageInfos, uint32_t stageCount) 
		: ShaderSet()
	{
		create(pStageInfos, stageCount);
	}

	ShaderSet::ShaderSet(const std::vector<ShaderStageInfo>& stageInfos) 
		: ShaderSet()
	{
		create(stageInfos);
	}
	
	ShaderSet::ShaderSet(const std::vector<std::vector<uint32_t>>& shaderCode) 
		: ShaderSet()
	{
		create(shaderCode);
	}

	bool ShaderSet::isValid() const {
		return !m_shaderModules.empty();
	}
	
	void ShaderSet::create(const ShaderStageInfo* pStageInfos, uint32_t stageCount) {
		if (isValid()) {
			throw std::runtime_error("Shader already created!");
			return;
		}

		for (int i = 0; i < stageCount; i++) {
			spirv_cross::Compiler compiler(pStageInfos[i].pCode, pStageInfos[i].codeLength);
			createShaderModule(pStageInfos[i]);
			initializeStage(&compiler, pStageInfos[i].stage);
		}

		createDescriptorPoolAndLayouts();
	}

	void ShaderSet::create(const std::vector<ShaderStageInfo>& stageInfos) {
		create(stageInfos.data(), stageInfos.size());
	}


	void ShaderSet::create(const std::vector<std::vector<uint32_t>>& shaderCode) {
		if (isValid()) {
			throw std::runtime_error("Shader already created!");
			return;
		}
		for (auto& code : shaderCode) {
			spirv_cross::Compiler compiler(code);
			const auto& entryPoints = compiler.get_entry_points_and_stages();

			ShaderStageFlagBits stage = ShaderStageFlagBits(1U << (uint32_t)entryPoints[0].execution_model);

			ShaderStageInfo stageInfo = {};
			stageInfo.pCode = (uint32_t*)code.data();
			stageInfo.codeLength = code.size();
			stageInfo.pName = (char*)entryPoints[0].name.c_str();
			stageInfo.stage = stage;
			createShaderModule(stageInfo);

			initializeStage(&compiler, stage);

		}

		createDescriptorPoolAndLayouts();
	}

	void ShaderSet::destroy() {

		for (const auto& [set, layout] : m_descriptorSetLayouts) {
			ResourceManager::get().remove<vk::DescriptorSetLayout>(layout);
		}

		if (m_descriptorPool != NULL_RESOURCE) {
			ResourceManager::get().remove<vk::DescriptorPool>(m_descriptorPool);
		}

		for (const auto& shader: m_shaderModules) {
			ResourceManager::get().remove<vk::ShaderModule>(shader.moduleID);
		}

		m_descriptorPool = NULL_RESOURCE;

		m_descriptorSetLayoutInfos.clear();
		m_descriptorSetLayouts.clear();
		m_shaderModules.clear();
		m_pushConstantRanges.clear();
		m_attributes.clear();
		m_vertexAttributes.clear();
		m_vertexBindings.clear();
	}

	const std::map<uint32_t, ResourceID>& ShaderSet::getDescriptorSetLayouts() const {
		return m_descriptorSetLayouts;
	}

	const std::vector<PushConstantRange>& ShaderSet::getPushConstantRanges() const {
		return m_pushConstantRanges;
	}

	const std::vector<ShaderModuleInfo>& ShaderSet::getShaderModules() const {
		return m_shaderModules;
	}

	const std::vector<VertexInputAttributeDescription>& ShaderSet::getVertexAttributes() const {
		return m_vertexAttributes;
	}

	const std::vector<VertexInputBindingDescription>& ShaderSet::getVertexBindings() const {
		return m_vertexBindings;
	}

	const ShaderAttribute& ShaderSet::getShaderAttribute(const std::string& attributePath) const {
		return m_attributes.at(attributePath);
	}

	const std::unordered_map<std::string, ShaderAttribute>& ShaderSet::getShaderAttributes() const {
		return m_attributes;
	}

	bool ShaderSet::isGraphicsSet() const {
		return m_isGraphicsSet;
	}

	bool ShaderSet::hasTessellationStage() const {
		return m_hasTessellationStage;
	}

	ResourceID ShaderSet::allocateDescriptorSet(uint32_t setIndex) const {
		if (!m_descriptorSetLayouts.count(setIndex)) {
			throw std::runtime_error("Invalid set index!");
		}
		
		vk::DescriptorPool* pDescriptorPool = ResourceManager::get().get<vk::DescriptorPool>(m_descriptorPool);
		vk::DescriptorSetLayout* pLayout = ResourceManager::get().get<vk::DescriptorSetLayout>(m_descriptorSetLayouts.at(setIndex));
		
		if (!pDescriptorPool) {
			SA_DEBUG_LOG_ERROR("Invalid descriptor pool ID", m_descriptorPool);
			throw std::runtime_error("Invalid descriptor pool ID!");
		}

		if (!pLayout) {
			SA_DEBUG_LOG_ERROR("Invalid descriptor layout ID", m_descriptorSetLayouts.at(setIndex));
			throw std::runtime_error("Invalid descriptor layout ID!");
		}

		DescriptorSet descriptorSet;
		descriptorSet.create(m_pCore->getDevice(), *pDescriptorPool, m_pCore->getQueueCount(), m_descriptorSetLayoutInfos.at(setIndex), *pLayout, setIndex);

		return ResourceManager::get().insert<DescriptorSet>(descriptorSet);
	}

	ShaderAttribute::~ShaderAttribute() {
		SA_DEBUG_LOG_INFO("Destroyed attribute ", name);
	}

}
