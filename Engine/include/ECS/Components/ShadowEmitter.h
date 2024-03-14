#pragma once
#include "../ComponentBase.h"
#include "Resources/Texture.hpp"

namespace comp {

	struct ShadowEmitter : public sa::ComponentBase {
		sa::Texture2D shadowmaps[4];
		uint32_t shadowMapCount;
		
		virtual void serialize(sa::Serializer& s) override;
		virtual void deserialize(void* pDoc) override;

		virtual void onConstruct(sa::Entity* e) override;
		virtual void onUpdate(sa::Entity* e) override;
		virtual void onDestroy(sa::Entity* e) override;
		//virtual void onCopy(sa::Entity* e, sa::Entity* other) {};

	};

}
