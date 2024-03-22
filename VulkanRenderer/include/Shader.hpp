#pragma once
#include "DescriptorSetStructs.h"
#include "ApiBuildOptions.h"

namespace sa {
	class VulkanCore;

	class Shader {
	private:
		VulkanCore* m_pCore;

		ResourceID m_shader;
	public:
		Shader();

		void create(const std::vector<uint32_t> &shaderCode);

		ResourceID getShaderID() const;
	};

}