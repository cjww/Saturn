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
		Color color = SA_COLOR_WHITE;	// 16 bytes
		sa::Vector3 position = sa::Vector3(0);		// 12 bytes - 16 offset
		float intensity = 1.0f;						// 4 bytes - 28 offset
		float attenuationRadius = 7.0f;
		LightType type = LightType::POINT;
	};

}

namespace comp {
	struct Light : public sa::LuaAccessable {
		unsigned int index;
		sa::LightData values;

		Light() = default;


		virtual void serialize(sa::Serializer& s) override;
		virtual void deserialize(void* pDoc) override;


		static void reg();
	};
}