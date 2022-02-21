#pragma once
#include <ResourceManager.h>
#include "Tools\Vector.h"


// Forward declarations
namespace vr {
	struct Buffer;
	struct DescriptorSet;
	typedef std::shared_ptr<DescriptorSet> DescriptorSetPtr;

}

namespace sa {
	namespace details {
		class EntityEnvironment;
	}
}

namespace comp {

	struct Name {
		std::string name;
	};

	struct Model {
		ResourceID modelID = NULL_RESOURCE;
		vr::Buffer* buffer = nullptr;
		vr::DescriptorSetPtr descriptorSet = nullptr;
	};

	struct Transform {
		sa::Vector3 position = sa::Vector3(0);
		sa::Vector3 rotation = sa::Vector3(0);
		sa::Vector3 scale = sa::Vector3(1);
	};

	struct Script {
		//sol::environment env;
		std::unique_ptr<sa::details::EntityEnvironment> env = nullptr;
	};

	enum class LightType {
		POINT = 0,
		DIRECTIONAL = 1
	};

	struct Light {
		glm::vec3 color = glm::vec3(1);
		float strength = 1.0f;
		LightType type = LightType::POINT;
	};

};

