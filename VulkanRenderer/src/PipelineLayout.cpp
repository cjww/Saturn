#include "pch.h"
#include "PipelineLayout.hpp"
#include "internal/VulkanCore.hpp"
#include "internal/DescriptorSet.hpp"

#include "Renderer.hpp"

#define NOMINMAX
#include <spirv_cross/spirv_cross.hpp>

#include <shaderc/shaderc.hpp>
#include <filesystem>

#include "internal/RenderProgram.hpp"


namespace sa {
	DescriptorSetFactory::DescriptorSetFactory(PipelineLayout* pLayout, uint32_t setIndex)
		: m_playout(pLayout)
		, m_setIndex(setIndex)
	{
		
	}

	DescriptorSetFactory& DescriptorSetFactory::addBinding(uint32_t binding, DescriptorType type, uint32_t count, ShaderStageFlags stageFlags, size_t size) {
		DescriptorSetLayoutBinding bindingInfo = {};
		bindingInfo.type = type;
		bindingInfo.binding = binding;
		bindingInfo.descriptorCount = count;
		bindingInfo.pImmutableSamplers = nullptr;
		bindingInfo.stageFlags = stageFlags;
		
		m_set.bindings.push_back(bindingInfo);
		m_set.sizes.push_back(size);
		return *this;
	}

	void DescriptorSetFactory::endDescriptorSet() const {
		m_playout->m_descriptorSetLayoutInfos[m_setIndex] = m_set;
	}

	shaderc_shader_kind ToShadercKind(ShaderStageFlagBits shaderStage) {
		static const std::unordered_map<ShaderStageFlagBits, shaderc_shader_kind> map = {
			{ShaderStageFlagBits::VERTEX, shaderc_glsl_default_vertex_shader },
			{ShaderStageFlagBits::FRAGMENT, shaderc_glsl_default_fragment_shader},
			{ShaderStageFlagBits::COMPUTE, shaderc_glsl_default_compute_shader},
			{ShaderStageFlagBits::GEOMETRY, shaderc_glsl_default_geometry_shader},
			{ShaderStageFlagBits::TESSELLATION_CONTROL, shaderc_glsl_default_tess_control_shader},
			{ShaderStageFlagBits::TESSELLATION_EVALUATION, shaderc_glsl_default_tess_evaluation_shader},
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
		if (!attributePath.empty())
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
			if ((type == DescriptorType::SAMPLED_IMAGE || type == DescriptorType::COMBINED_IMAGE_SAMPLER || type == DescriptorType::STORAGE_IMAGE) && t.image.dim == spv::Dim::DimBuffer) {
				continue; // this is not a sampled image
			}
			if ((type == DescriptorType::UNIFORM_TEXEL_BUFFER || type == DescriptorType::STORAGE_TEXEL_BUFFER) && t.image.dim != spv::Dim::DimBuffer) {
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
					layoutBinding.descriptorCount = VARIABLE_DESCRIPTOR_COUNT;
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

	class ShaderIncluder : public shaderc::CompileOptions::IncluderInterface {
	private:
		std::filesystem::path m_additionalIncludeDirectory;

	public:
		ShaderIncluder(const char* additionalIncludeDirectory) : shaderc::CompileOptions::IncluderInterface() {
			m_additionalIncludeDirectory = additionalIncludeDirectory;
		}

		shaderc_include_result* GetInclude(
			const char* requested_source,
			shaderc_include_type type,
			const char* requesting_source,
			size_t include_depth)
		{

			std::filesystem::path sourceName = requested_source;
			std::filesystem::path includer = requesting_source;

			auto source = includer.parent_path() / sourceName;
			if (!std::filesystem::exists(source)) {
				// check include dir
				source =  m_additionalIncludeDirectory / sourceName;
			}

			const std::string contents = ReadFile(source.generic_string().c_str());

			auto container = new std::array<std::string, 2>;
			(*container)[0] = source.generic_string();
			(*container)[1] = contents;

			auto data = new shaderc_include_result;

			data->user_data = container;

			data->source_name = (*container)[0].data();
			data->source_name_length = (*container)[0].size();

			data->content = (*container)[1].data();
			data->content_length = (*container)[1].size();

			return data;
		};

		void ReleaseInclude(shaderc_include_result* data) override {
			delete static_cast<std::array<std::string, 2>*>(data->user_data);
			delete data;
		};
	};

	std::vector<uint32_t> CompileGLSLFromFile(const char* glslPath, ShaderStageFlagBits shaderStage, const char* entryPointName, const char* additionalIncludeDirectory) {
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

		return CompileGLSLFromMemory(buffer.c_str(), shaderStage, entryPointName, additionalIncludeDirectory, glslPath);
	}

	std::vector<uint32_t> CompileGLSLFromMemory(const char* glslCode, ShaderStageFlagBits shaderStage, const char* entryPointName, const char* additionalIncludeDirectory, const char* tag) {
		shaderc::CompileOptions options;
		options.SetAutoBindUniforms(true);
		options.SetAutoMapLocations(true);
		options.SetTargetEnvironment(shaderc_target_env_vulkan, SA_VK_API_VERSION);
		options.SetIncluder(std::make_unique<ShaderIncluder>(additionalIncludeDirectory));

		shaderc::Compiler compiler;
		shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(glslCode, ToShadercKind(shaderStage), tag, entryPointName, options);

		if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
			throw std::runtime_error("Failed to compile GLSL code: " + result.GetErrorMessage());
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

	std::string ReadFile(const char* path) {
		std::ifstream file(path);
		if (!file.is_open()) {
			throw std::runtime_error("Failed to open file " + std::string(path));
		}
		file.ignore(std::numeric_limits<std::streamsize>::max());
		std::streamsize length = file.gcount();
		file.clear();
		file.seekg(0, std::ios_base::beg);

		std::string buffer;
		buffer.resize(length);
		file.read(buffer.data(), length);

		file.close();

		return std::move(buffer);
	}

	void PipelineLayout::initializeStage(const ShaderStageInfo& stageInfo) {
		spirv_cross::Compiler compiler(stageInfo.pCode, stageInfo.codeLength);
		initializeStage(&compiler, stageInfo.stage);
	}

	void PipelineLayout::initializeStage(spirv_cross::Compiler* pCompiler, ShaderStageFlagBits stage) {
		if ((stage & sa::ShaderStageFlagBits::COMPUTE) != 0)
			m_isGraphicsPipeline = false;
		if ((stage & (sa::ShaderStageFlagBits::TESSELLATION_CONTROL | sa::ShaderStageFlagBits::TESSELLATION_EVALUATION)) != 0)
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

	void PipelineLayout::createDescriptorPoolAndLayouts() {
		std::set<vk::DescriptorType> descriptorTypes;
		std::vector<vk::DescriptorPoolSize> poolSizes;
		for (auto& [set, info] : m_descriptorSetLayoutInfos) {




			vk::DescriptorSetLayoutBindingFlagsCreateInfo flagCreateInfo;
			std::vector<vk::DescriptorBindingFlags> flags(info.bindings.size(), (vk::DescriptorBindingFlags)0);

			

			std::vector<vk::DescriptorSetLayoutBinding> layoutBindings(info.bindings.size());
			for (int i = 0; i < layoutBindings.size(); i++) {
				layoutBindings[i].descriptorCount = info.bindings[i].descriptorCount;

				if (info.bindings[i].descriptorCount > 1) {
					flags[i] |= vk::DescriptorBindingFlagBits::ePartiallyBound; // allow use to not bind all descriptors if not needed
				}
				else if (info.bindings[i].descriptorCount == VARIABLE_DESCRIPTOR_COUNT) {
					// to support variable descriptor counts
					layoutBindings[i].descriptorCount = MAX_VARIABLE_DESCRIPTOR_COUNT;
					flags[i] = vk::DescriptorBindingFlagBits::eVariableDescriptorCount | vk::DescriptorBindingFlagBits::ePartiallyBound;
					auto it = std::max_element(info.bindings.begin(), info.bindings.end(), [](const auto& highest, const auto& next) { return highest.binding < next.binding; });
					if (info.bindings[i].binding != it->binding) {
						throw std::runtime_error("Variable count descriptors has to be on the last binding");
					}
				}

				layoutBindings[i].binding = info.bindings[i].binding;
				layoutBindings[i].descriptorType = (vk::DescriptorType)info.bindings[i].type;
				layoutBindings[i].pImmutableSamplers = info.bindings[i].pImmutableSamplers;
				layoutBindings[i].stageFlags = (vk::ShaderStageFlags)info.bindings[i].stageFlags;
			}
			flagCreateInfo.setBindingFlags(flags);
			
			vk::DescriptorSetLayoutCreateInfo layoutInfo;
			layoutInfo.setPNext(&flagCreateInfo);
			layoutInfo.setBindings(layoutBindings);

			m_descriptorSetLayouts[set] = ResourceManager::Get().insert<vk::DescriptorSetLayout>(m_pCore->getDevice().createDescriptorSetLayout(layoutInfo));

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
			m_descriptorPool = ResourceManager::Get().insert<vk::DescriptorPool>(m_pCore->getDevice().createDescriptorPool(poolInfo));
		}
		
	}

	void PipelineLayout::createPipelineLayout() {
		vk::PipelineLayoutCreateInfo layoutInfo = {};

		const auto& pushCostantRanges = getPushConstantRanges();

		std::vector<vk::PushConstantRange> vk_pushConstantRanges(pushCostantRanges.size());
		for (int i = 0; i < vk_pushConstantRanges.size(); i++) {
			vk_pushConstantRanges[i].offset = pushCostantRanges[i].offset;
			vk_pushConstantRanges[i].stageFlags = static_cast<vk::ShaderStageFlags>(pushCostantRanges[i].stageFlags);
			vk_pushConstantRanges[i].size = pushCostantRanges[i].size;
		}

		const auto& descriptorSetLayouts = getDescriptorSetLayouts();
		std::vector<vk::DescriptorSetLayout> vk_descriptorSetLayouts;

		if (!descriptorSetLayouts.empty()) {

			auto back = descriptorSetLayouts.end();
			--back;
			uint32_t maxSet = back->first;

			for (uint32_t i = 0; i <= maxSet; i++) {
				if (!descriptorSetLayouts.count(i)) {
					SA_DEBUG_LOG_ERROR("Set index ", i, " does not exist! All sets from 0 to ", maxSet, " must be used!");
					throw std::runtime_error("Set index does not exist");
				}
				const vk::DescriptorSetLayout* pLayout = ResourceManager::Get().get<vk::DescriptorSetLayout>(descriptorSetLayouts.at(i));
				if (!pLayout) {
					SA_DEBUG_LOG_ERROR("Invalid descriptor layout ID ", descriptorSetLayouts.at(i));
					continue;
				}
				vk_descriptorSetLayouts.push_back(*pLayout);
			}
		}

		layoutInfo.setPushConstantRanges(vk_pushConstantRanges);
		layoutInfo.setSetLayouts(vk_descriptorSetLayouts);
		vk::PipelineLayout l = m_pCore->getDevice().createPipelineLayout(layoutInfo);
		m_layout = ResourceManager::Get().insert(l);
	}

	PipelineLayout::~PipelineLayout() {
		if(m_layout != NULL_RESOURCE)
			ResourceManager::Get().remove<vk::PipelineLayout>(m_layout);
	}

	DescriptorSetFactory PipelineLayout::beginDescriptorSet(uint32_t setIndex) {
		return DescriptorSetFactory(this, setIndex);
	}

	PipelineLayout::PipelineLayout()
		: m_pCore(Renderer::Get().getCore())
		, m_layout(NULL_RESOURCE)
		, m_descriptorPool(NULL_RESOURCE)
		, m_isGraphicsPipeline(true)
		, m_hasTessellationStage(false)
	{
	}

	void PipelineLayout::create() {
		createPipelineLayout();
	}

	void PipelineLayout::createFromShaders(const ShaderStageInfo* pStageInfos, uint32_t stageCount) {
		
		for (int i = 0; i < stageCount; i++) {
			spirv_cross::Compiler compiler(pStageInfos[i].pCode, pStageInfos[i].codeLength);
			initializeStage(&compiler, pStageInfos[i].stage);
		}

		createDescriptorPoolAndLayouts();

		createPipelineLayout();

	}

	void PipelineLayout::createFromShaders(const std::vector<ShaderStageInfo>& stageInfos) {
		createFromShaders(stageInfos.data(), stageInfos.size());
	}


	void PipelineLayout::createFromShaders(const std::vector<std::vector<uint32_t>>& shaderCode) {
		for (auto& code : shaderCode) {
			spirv_cross::Compiler compiler(code);
			const auto& entryPoints = compiler.get_entry_points_and_stages();

			ShaderStageFlagBits stage = ShaderStageFlagBits(1U << (uint32_t)entryPoints[0].execution_model);

			initializeStage(&compiler, stage);

		}
		createDescriptorPoolAndLayouts();
		createPipelineLayout();
	}

	void PipelineLayout::createFromShaders(const std::vector<uint32_t>& shaderCode) {
		spirv_cross::Compiler compiler(shaderCode);
		const auto& entryPoints = compiler.get_entry_points_and_stages();

		ShaderStageFlagBits stage = ShaderStageFlagBits(1U << (uint32_t)entryPoints[0].execution_model);

		initializeStage(&compiler, stage);
		
		createDescriptorPoolAndLayouts();
		createPipelineLayout();
	}

	void PipelineLayout::createFromShaders(const std::vector<Shader>& shaders) {
		createFromShaders(shaders.data(), static_cast<uint32_t>(shaders.size()));
	}

	void PipelineLayout::createFromShaders(const Shader* pShaders, uint32_t shaderCount) {
		if(shaderCount > 8) 
			throw std::runtime_error("shaderCount bigger than buffer");
		
		std::array<ShaderStageInfo, 8> infos;
		for(uint32_t i = 0; i < shaderCount && i < infos.size(); i++) {
			infos[i] = pShaders[i].getShaderStageInfo();
		}
		createFromShaders(infos.data(), shaderCount);
	}

	void PipelineLayout::destroy() {
		for (auto id : m_allocatedDescriptorSets) {
			ResourceManager::Get().remove<DescriptorSet>(id);
		}
		m_allocatedDescriptorSets.clear();
		std::set<ResourceID> allocatedDescSets;
		m_allocatedDescriptorSets.swap(allocatedDescSets);

		for (const auto& [set, layout] : m_descriptorSetLayouts) {
			ResourceManager::Get().remove<vk::DescriptorSetLayout>(layout);
		}

		if (m_descriptorPool != NULL_RESOURCE) {
			ResourceManager::Get().remove<vk::DescriptorPool>(m_descriptorPool);
		}
		m_descriptorPool = NULL_RESOURCE;

		if(m_layout != NULL_RESOURCE) {
			ResourceManager::Get().remove<vk::PipelineLayout>(m_layout);
		}
		m_layout = NULL_RESOURCE;

		m_descriptorSetLayoutInfos.clear();
		std::unordered_map<uint32_t, DescriptorSetLayoutInfo> descSetInfo;
		m_descriptorSetLayoutInfos.swap(descSetInfo);

		std::map<uint32_t, ResourceID> descSetLayouts;
		m_descriptorSetLayouts.clear();
		m_descriptorSetLayouts.swap(descSetLayouts);

		m_pushConstantRanges.clear();
		m_pushConstantRanges.shrink_to_fit();

		std::unordered_map<std::string, ShaderAttribute> attribs;
		m_attributes.clear();
		m_attributes.swap(attribs);

		m_vertexAttributes.clear();
		m_vertexAttributes.shrink_to_fit();
		m_vertexBindings.clear();
		m_vertexBindings.shrink_to_fit();

	}

	bool PipelineLayout::isValid() const {
		return m_layout != NULL_RESOURCE;
	}

	const std::map<uint32_t, ResourceID>& PipelineLayout::getDescriptorSetLayouts() const {
		return m_descriptorSetLayouts;
	}

	const std::vector<PushConstantRange>& PipelineLayout::getPushConstantRanges() const {
		return m_pushConstantRanges;
	}
	
	const std::vector<VertexInputAttributeDescription>& PipelineLayout::getVertexAttributes() const {
		return m_vertexAttributes;
	}

	const std::vector<VertexInputBindingDescription>& PipelineLayout::getVertexBindings() const {
		return m_vertexBindings;
	}

	const ShaderAttribute& PipelineLayout::getShaderAttribute(const std::string& attributePath) const {
		return m_attributes.at(attributePath);
	}

	const std::unordered_map<std::string, ShaderAttribute>& PipelineLayout::getShaderAttributes() const {
		return m_attributes;
	}
	
	bool PipelineLayout::isGraphicsPipeline() const {
		return m_isGraphicsPipeline;
	}

	bool PipelineLayout::hasTessellationStage() const {
		return m_hasTessellationStage;
	}

	ResourceID PipelineLayout::allocateDescriptorSet(uint32_t setIndex) {
		if (!m_descriptorSetLayouts.count(setIndex)) {
			throw std::runtime_error("Invalid set index!");
		}

		vk::DescriptorPool* pDescriptorPool = ResourceManager::Get().get<vk::DescriptorPool>(m_descriptorPool);
		vk::DescriptorSetLayout* pLayout = ResourceManager::Get().get<vk::DescriptorSetLayout>(m_descriptorSetLayouts.at(setIndex));

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
		ResourceID id = ResourceManager::Get().insert<DescriptorSet>(descriptorSet);
		m_allocatedDescriptorSets.insert(id);
		return id;
	}

	bool PipelineLayout::hasAllocatedDescriptorSet(ResourceID descriptorSet) {
		return m_allocatedDescriptorSets.contains(descriptorSet);
	}
	
	ResourceID PipelineLayout::getLayoutID() const {
		return m_layout;
	}
	
}
