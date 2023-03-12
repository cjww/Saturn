#pragma once

#include "Tools\Vector.h"

#include "ECS/ComponentBase.h"

namespace sa {

	enum class LightType : uint32_t {
		POINT = 0,
		DIRECTIONAL = 1,
		SPOT = 2,
	};

	struct alignas(16) LightData {
		Color color = SA_COLOR_WHITE;					// vec3 rgb, float intensity
		glm::vec4 position = glm::vec4(0, 0, 0, 7);		// vec3 position, float attenuationRadius
		glm::vec4 direction = glm::vec4(1, 0, 0, 0);	// vec3 direction,
		LightType type = LightType::POINT;
	};

}

namespace comp {
	class Light : public sa::LuaAccessable {
	public:
		unsigned int index;
		sa::LightData values;

		Light() = default;


		virtual void serialize(sa::Serializer& s) override;
		virtual void deserialize(void* pDoc) override;


		static void reg();
	};
}