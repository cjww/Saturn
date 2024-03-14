#include "pch.h"
#include "ECS/Components/ShadowEmitter.h"

void comp::ShadowEmitter::serialize(sa::Serializer& s) {

}

void comp::ShadowEmitter::deserialize(void* pDoc) {

}

void comp::ShadowEmitter::onConstruct(sa::Entity* e) {
	
	shadowmaps[0] = sa::Texture2D(sa::TextureTypeFlagBits::DEPTH_ATTACHMENT | sa::TextureTypeFlagBits::SAMPLED, sa::Extent{ 256, 256 });
	shadowMapCount = 1;
}

void comp::ShadowEmitter::onUpdate(sa::Entity* e) {
	if (shadowMapCount != 0) {
		for (int i = 0; i < shadowMapCount; i++) {
			shadowmaps[i].destroy();
		}
	}
	shadowmaps[0] = sa::Texture2D(sa::TextureTypeFlagBits::DEPTH_ATTACHMENT | sa::TextureTypeFlagBits::SAMPLED, sa::Extent{ 256, 256 });
	shadowMapCount = 1;
}

void comp::ShadowEmitter::onDestroy(sa::Entity* e) {
	for (int i = 0; i < shadowMapCount; i++) {
		shadowmaps[i].destroy();
	}
}
